// Copyright (c) Yevhenii Selivanov

#include "K2Node_BindAndLoadRegistryRows.h"

// DAL
#include "DalRegistrySubsystem.h"

// UE
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_Self.h"
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_BindAndLoadRegistryRows)

#define LOCTEXT_NAMESPACE "K2Node_BindAndLoadRegistryRows"

// Returns the name of the native function to call
FName UK2Node_BindAndLoadRegistryRows::GetNativeFunctionName()
{
	return GET_FUNCTION_NAME_CHECKED(UDalRegistrySubsystem, BPBindAndLoad);
}

// UEdGraphNode interface

void UK2Node_BindAndLoadRegistryRows::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	UEdGraphPin* CompletedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletedPinName);
	CompletedPin->PinToolTip = LOCTEXT("CompletedTooltip", "Called when Data Registry rows change and all soft references finish async loading").ToString();

	UEdGraphPin* RowStructPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UScriptStruct::StaticClass(), RowStructPinName);
	RowStructPin->PinToolTip = LOCTEXT("RowStructPinTooltip", "The row struct type to subscribe to in the Data Registry").ToString();
}

FText UK2Node_BindAndLoadRegistryRows::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Subscribes to Data Registry cache changes for the specified row struct type and async loads soft references. Fires Completed each time rows change and loading finishes.");
}

FText UK2Node_BindAndLoadRegistryRows::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Bind And Load Registry Rows");
}

bool UK2Node_BindAndLoadRegistryRows::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	const EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	const bool bIsCompatible = GraphType == GT_Ubergraph || GraphType == GT_Macro;
	return bIsCompatible && Super::IsCompatibleWithGraph(TargetGraph);
}

// UK2Node interface

void UK2Node_BindAndLoadRegistryRows::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(Schema);
	bool bIsErrorFree = true;

	// Sequence node for ordered execution
	UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();

	// Connect input exec to sequence
	{
		UEdGraphPin* InputExePin = GetExecPin();
		UEdGraphPin* SequenceInputExePin = SequenceNode->GetExecPin();
		bIsErrorFree &= InputExePin && SequenceInputExePin && CompilerContext.MovePinLinksToIntermediate(*InputExePin, *SequenceInputExePin).CanSafeConnect();
	}

	// Create BPBindAndLoad function call
	UK2Node_CallFunction* CallBindNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallBindNode->FunctionReference.SetExternalMember(GetNativeFunctionName(), UDalRegistrySubsystem::StaticClass());
	CallBindNode->AllocateDefaultPins();

	// Connect call to first sequence pin
	{
		UEdGraphPin* CallFunctionInputExePin = CallBindNode->GetExecPin();
		UEdGraphPin* SequenceFirstExePin = SequenceNode->GetThenPinGivenIndex(0);
		bIsErrorFree &= SequenceFirstExePin && CallFunctionInputExePin && Schema->TryCreateConnection(CallFunctionInputExePin, SequenceFirstExePin);
	}

	// Connect Self node as Owner parameter
	UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
	SelfNode->AllocateDefaultPins();
	{
		static const FName OwnerParamName = TEXT("Owner");
		UEdGraphPin* CallOwnerPin = CallBindNode->FindPin(OwnerParamName);
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		bIsErrorFree &= CallOwnerPin && SelfPin && Schema->TryCreateConnection(CallOwnerPin, SelfPin);
	}

	// Connect RowStruct input
	{
		static const FName RowStructParamName = TEXT("RowStruct");
		UEdGraphPin* CallRowStructPin = CallBindNode->FindPin(RowStructParamName);
		UEdGraphPin* RowStructPin = FindPin(RowStructPinName);
		if (RowStructPin && CallRowStructPin)
		{
			if (RowStructPin->LinkedTo.Num() > 0)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*RowStructPin, *CallRowStructPin).CanSafeConnect();
			}
			else
			{
				CallRowStructPin->DefaultValue = RowStructPin->DefaultValue;
				CallRowStructPin->DefaultObject = RowStructPin->DefaultObject;
			}
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	// Create OnChanged delegate via custom event
	UK2Node_CustomEvent* ChangedEventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	ChangedEventNode->CustomFunctionName = *FString::Printf(TEXT("%s_%s"), *CompletedPinName.ToString(), *CompilerContext.GetGuid(this));
	ChangedEventNode->AllocateDefaultPins();

	// Connect delegate
	{
		static const FName OnChangedParamName = TEXT("OnChanged");
		UEdGraphPin* CallFunctionDelegatePin = CallBindNode->FindPin(OnChangedParamName);
		UEdGraphPin* EventDelegatePin = ChangedEventNode->FindPin(UK2Node_CustomEvent::DelegateOutputName);
		bIsErrorFree &= CallFunctionDelegatePin && EventDelegatePin && Schema->TryCreateConnection(CallFunctionDelegatePin, EventDelegatePin);
	}

	// Connect event output to Completed pin
	{
		UEdGraphPin* EventOutputExePin = ChangedEventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* OutputCompletedPin = FindPin(CompletedPinName);
		bIsErrorFree &= EventOutputExePin && OutputCompletedPin && CompilerContext.MovePinLinksToIntermediate(*OutputCompletedPin, *EventOutputExePin).CanSafeConnect();
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "K2Node_BindAndLoadRegistryRows: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

FName UK2Node_BindAndLoadRegistryRows::GetCornerIcon() const
{
	return TEXT("Graph.Latent.LatentIcon");
}

void UK2Node_BindAndLoadRegistryRows::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BindAndLoadRegistryRows::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Data Assets Loader");
}

#undef LOCTEXT_NAMESPACE
