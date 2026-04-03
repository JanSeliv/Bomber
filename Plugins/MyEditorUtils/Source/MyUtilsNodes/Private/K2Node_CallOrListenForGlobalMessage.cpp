// Copyright (c) Yevhenii Selivanov

#include "K2Node_CallOrListenForGlobalMessage.h"

// MyUtils
#include "Subsystems/GlobalMessageSubsystem.h"

// UE
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_CallOrListenForGlobalMessage)

#define LOCTEXT_NAMESPACE "K2Node_CallOrListenForGlobalMessage"

// Returns the name of the native function to call
FName UK2Node_CallOrListenForGlobalMessage::GetNativeFunctionName()
{
	return GET_FUNCTION_NAME_CHECKED(UGlobalMessageSubsystem, BPCallOrStartListeningForGlobalMessage);
}

// UEdGraphNode interface

void UK2Node_CallOrListenForGlobalMessage::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	UEdGraphPin* OnMessageReceivedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, OnMessageReceivedPinName);
	OnMessageReceivedPin->PinToolTip = LOCTEXT("OnMessageReceivedTooltip", "Fires when a matching global message is received, or immediately if the event was already broadcast").ToString();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FGameplayTag::StaticStruct(), MessageTagPinName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, FGameplayEventData::StaticStruct(), PayloadPinName);
}

FText UK2Node_CallOrListenForGlobalMessage::GetTooltipText() const
{
	const UFunction* Function = UGlobalMessageSubsystem::StaticClass()->FindFunctionByName(GetNativeFunctionName());
	return Function ? Function->GetToolTipText() : Super::GetTooltipText();
}

FText UK2Node_CallOrListenForGlobalMessage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UFunction* Function = UGlobalMessageSubsystem::StaticClass()->FindFunctionByName(GetNativeFunctionName());
	return Function ? Function->GetDisplayNameText() : Super::GetNodeTitle(TitleType);
}

bool UK2Node_CallOrListenForGlobalMessage::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	const EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	const bool bIsCompatible = GraphType == GT_Ubergraph || GraphType == GT_Macro;
	return bIsCompatible && Super::IsCompatibleWithGraph(TargetGraph);
}

// UK2Node interface

void UK2Node_CallOrListenForGlobalMessage::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(Schema);
	bool bIsErrorFree = true;

	// Create BPCallOrStartListeningForGlobalMessage function call
	UK2Node_CallFunction* CallListenNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallListenNode->FunctionReference.SetExternalMember(GetNativeFunctionName(), UGlobalMessageSubsystem::StaticClass());
	CallListenNode->AllocateDefaultPins();

	// Connect input exec
	{
		UEdGraphPin* InputExePin = GetExecPin();
		UEdGraphPin* CallExePin = CallListenNode->GetExecPin();
		bIsErrorFree &= InputExePin && CallExePin && CompilerContext.MovePinLinksToIntermediate(*InputExePin, *CallExePin).CanSafeConnect();
	}

	// Connect MessageTag input
	{
		UEdGraphPin* OurTagPin = FindPin(MessageTagPinName);
		UEdGraphPin* CallTagPin = CallListenNode->FindPin(MessageTagPinName);
		if (OurTagPin && CallTagPin)
		{
			if (OurTagPin->LinkedTo.Num() > 0)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OurTagPin, *CallTagPin).CanSafeConnect();
			}
			else
			{
				CallTagPin->DefaultValue = OurTagPin->DefaultValue;
			}
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	// Create custom event for delegate callback
	UK2Node_CustomEvent* CompletedEventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	CompletedEventNode->CustomFunctionName = *FString::Printf(TEXT("OnGlobalMessage_%s"), *CompilerContext.GetGuid(this));
	CompletedEventNode->AllocateDefaultPins();

	// Add delegate parameters to custom event from the Completed delegate signature
	{
		const UFunction* ListenFunction = CallListenNode->GetTargetFunction();
		const FDelegateProperty* CompletedDelegateProperty = ListenFunction ? FindFProperty<FDelegateProperty>(ListenFunction, CompletedPinName) : nullptr;
		const UFunction* CompletedSignature = CompletedDelegateProperty ? CompletedDelegateProperty->SignatureFunction : nullptr;
		ensureMsgf(CompletedSignature, TEXT("ASSERT: [%i] %hs:\n'CompletedSignature' is not valid!"), __LINE__, __FUNCTION__);
		for (TFieldIterator<FProperty> PropIt(CompletedSignature); PropIt && PropIt->PropertyFlags & CPF_Parm; ++PropIt)
		{
			const FProperty* Param = *PropIt;
			if (!Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm))
			{
				FEdGraphPinType PinType;
				bIsErrorFree &= Schema->ConvertPropertyToPinType(Param, /*out*/ PinType);
				bIsErrorFree &= CompletedEventNode->CreateUserDefinedPin(Param->GetFName(), PinType, EGPD_Output) != nullptr;
			}
		}
	}

	// Connect Then output exec
	{
		UEdGraphPin* CallThenPin = CallListenNode->GetThenPin();
		UEdGraphPin* OurThenPin = FindPin(UEdGraphSchema_K2::PN_Then);
		bIsErrorFree &= CallThenPin && OurThenPin && CompilerContext.MovePinLinksToIntermediate(*OurThenPin, *CallThenPin).CanSafeConnect();
	}

	// Connect delegate
	{
		UEdGraphPin* CallDelegatePin = CallListenNode->FindPin(CompletedPinName);
		UEdGraphPin* EventDelegatePin = CompletedEventNode->FindPin(UK2Node_CustomEvent::DelegateOutputName);
		bIsErrorFree &= CallDelegatePin && EventDelegatePin && Schema->TryCreateConnection(CallDelegatePin, EventDelegatePin);
	}

	// Connect output exec: custom event PN_Then → OnMessageReceived output
	{
		UEdGraphPin* EventThenPin = CompletedEventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* OurOnMessageReceivedPin = FindPin(OnMessageReceivedPinName);
		bIsErrorFree &= EventThenPin && OurOnMessageReceivedPin && CompilerContext.MovePinLinksToIntermediate(*OurOnMessageReceivedPin, *EventThenPin).CanSafeConnect();
	}

	// Connect output payload: custom event delegate param → MessagePayload output
	{
		static const FName DelegatePayloadParamName = TEXT("Payload");
		UEdGraphPin* EventPayloadPin = CompletedEventNode->FindPin(DelegatePayloadParamName);
		UEdGraphPin* OurPayloadPin = FindPin(PayloadPinName);
		bIsErrorFree &= EventPayloadPin && OurPayloadPin && CompilerContext.MovePinLinksToIntermediate(*OurPayloadPin, *EventPayloadPin).CanSafeConnect();
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "K2Node_CallOrListenForGlobalMessage: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

FName UK2Node_CallOrListenForGlobalMessage::GetCornerIcon() const
{
	return TEXT("Graph.Latent.LatentIcon");
}

void UK2Node_CallOrListenForGlobalMessage::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_CallOrListenForGlobalMessage::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Global Messages");
}

#undef LOCTEXT_NAMESPACE