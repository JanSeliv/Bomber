// Copyright (c) Yevhenii Selivanov

#include "K2Node_ListenForDataRegistryRow.h"

// DAL
#include "DalRegistrySubsystem.h"

// UE
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "DataRegistryId.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_Self.h"
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_ListenForDataRegistryRow)

#define LOCTEXT_NAMESPACE "K2Node_ListenForDataRegistryRow"

// Returns the name of the native function to call
FName UK2Node_ListenForDataRegistryRow::GetNativeFunctionName()
{
	return GET_FUNCTION_NAME_CHECKED(UDalRegistrySubsystem, BPListenForDataRegistryRow);
}

// UEdGraphNode interface

void UK2Node_ListenForDataRegistryRow::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	UEdGraphPin* CompletedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletedPinName);
	CompletedPin->PinToolTip = LOCTEXT("CompletedTooltip", "Called once the row and its soft references are resolvable in the Data Registry").ToString();

	UEdGraphPin* ItemIdPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FDataRegistryId::StaticStruct(), ItemIdPinName);
	ItemIdPin->PinToolTip = LOCTEXT("ItemIdPinTooltip", "The Data Registry item identifier (registry type + row name) to wait for").ToString();
}

FText UK2Node_ListenForDataRegistryRow::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "One-shot listener: fires Completed once the specified row and its soft references are resolvable in the Data Registry. Does not trigger async loading on its own.");
}

FText UK2Node_ListenForDataRegistryRow::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Listen For Data Registry Row");
}

bool UK2Node_ListenForDataRegistryRow::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	const EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	const bool bIsCompatible = GraphType == GT_Ubergraph || GraphType == GT_Macro;
	return bIsCompatible && Super::IsCompatibleWithGraph(TargetGraph);
}

// UK2Node interface

void UK2Node_ListenForDataRegistryRow::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
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

	// Create BPListenForDataRegistryRow function call
	UK2Node_CallFunction* CallListenNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallListenNode->FunctionReference.SetExternalMember(GetNativeFunctionName(), UDalRegistrySubsystem::StaticClass());
	CallListenNode->AllocateDefaultPins();

	// Connect call to first sequence pin
	{
		UEdGraphPin* CallFunctionInputExePin = CallListenNode->GetExecPin();
		UEdGraphPin* SequenceFirstExePin = SequenceNode->GetThenPinGivenIndex(0);
		bIsErrorFree &= SequenceFirstExePin && CallFunctionInputExePin && Schema->TryCreateConnection(CallFunctionInputExePin, SequenceFirstExePin);
	}

	// Connect Self node as Owner parameter
	UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
	SelfNode->AllocateDefaultPins();
	{
		static const FName OwnerParamName = TEXT("Owner");
		UEdGraphPin* CallOwnerPin = CallListenNode->FindPin(OwnerParamName);
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		bIsErrorFree &= CallOwnerPin && SelfPin && Schema->TryCreateConnection(CallOwnerPin, SelfPin);
	}

	// Connect ItemId input
	{
		static const FName ItemIdParamName = TEXT("ItemId");
		UEdGraphPin* CallItemIdPin = CallListenNode->FindPin(ItemIdParamName);
		UEdGraphPin* ItemIdPin = FindPin(ItemIdPinName);
		if (ItemIdPin && CallItemIdPin)
		{
			if (ItemIdPin->LinkedTo.Num() > 0)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*ItemIdPin, *CallItemIdPin).CanSafeConnect();
			}
			else
			{
				CallItemIdPin->DefaultValue = ItemIdPin->DefaultValue;
			}
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	// Create Completed delegate via custom event
	UK2Node_CustomEvent* CompletedEventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	CompletedEventNode->CustomFunctionName = *FString::Printf(TEXT("%s_%s"), *CompletedPinName.ToString(), *CompilerContext.GetGuid(this));
	CompletedEventNode->AllocateDefaultPins();

	// Connect delegate
	{
		static const FName CompletedParamName = TEXT("Completed");
		UEdGraphPin* CallFunctionDelegatePin = CallListenNode->FindPin(CompletedParamName);
		UEdGraphPin* EventDelegatePin = CompletedEventNode->FindPin(UK2Node_CustomEvent::DelegateOutputName);
		bIsErrorFree &= CallFunctionDelegatePin && EventDelegatePin && Schema->TryCreateConnection(CallFunctionDelegatePin, EventDelegatePin);
	}

	// Connect event output to Completed pin
	{
		UEdGraphPin* EventOutputExePin = CompletedEventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* OutputCompletedPin = FindPin(CompletedPinName);
		bIsErrorFree &= EventOutputExePin && OutputCompletedPin && CompilerContext.MovePinLinksToIntermediate(*OutputCompletedPin, *EventOutputExePin).CanSafeConnect();
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "K2Node_ListenForDataRegistryRow: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

FName UK2Node_ListenForDataRegistryRow::GetCornerIcon() const
{
	return TEXT("Graph.Latent.LatentIcon");
}

void UK2Node_ListenForDataRegistryRow::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_ListenForDataRegistryRow::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Data Assets Loader");
}

#undef LOCTEXT_NAMESPACE