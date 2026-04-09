// Copyright (c) Yevhenii Selivanov

#include "K2Node_GetAllRegistryRows.h"

// DAL
#include "DalUtilsLibrary.h"

// UE
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "Styling/AppStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_GetAllRegistryRows)

#define LOCTEXT_NAMESPACE "K2Node_GetAllRegistryRows"

namespace GetAllRegistryRowsHelper
{
	static const FName RowTypePinName = TEXT("RowType");
	static const FName OutRowsPinName = TEXT("OutRows");
} // namespace GetAllRegistryRowsHelper

// UEdGraphNode interface

void UK2Node_GetAllRegistryRows::AllocateDefaultPins()
{
	// RowType input: UScriptStruct* dropdown
	UEdGraphPin* RowTypePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UScriptStruct::StaticClass(), GetAllRegistryRowsHelper::RowTypePinName);
	RowTypePin->PinToolTip = LOCTEXT("RowTypePinTooltip", "The row struct type to query from the Data Registry").ToString();

	// OutRows output: TArray<SelectedStruct>, starts as wildcard
	UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, GetAllRegistryRowsHelper::OutRowsPinName);
	ResultPin->PinType.ContainerType = EPinContainerType::Array;
	ResultPin->PinToolTip = LOCTEXT("OutRowsPinTooltip", "All cached rows from the Data Registry for the selected struct type").ToString();

	Super::AllocateDefaultPins();
}

FText UK2Node_GetAllRegistryRows::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Get All Registry Rows");
}

FText UK2Node_GetAllRegistryRows::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Returns all cached rows from the Data Registry matching the selected row struct type");
}

void UK2Node_GetAllRegistryRows::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin && ChangedPin->PinName == GetAllRegistryRowsHelper::RowTypePinName)
	{
		RefreshOutputPinType();
	}
}

FSlateIcon UK2Node_GetAllRegistryRows::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = GetNodeTitleColor();
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
	return Icon;
}

// UK2Node interface

void UK2Node_GetAllRegistryRows::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_GetAllRegistryRows::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Data Assets Loader");
}

void UK2Node_GetAllRegistryRows::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* RowTypePin = GetRowTypePin();
	UEdGraphPin* ResultPin = GetResultPin();

	// Resolve the selected struct from the RowType pin
	const UScriptStruct* SelectedStruct = RowTypePin ? Cast<UScriptStruct>(RowTypePin->DefaultObject) : nullptr;
	if (!SelectedStruct && RowTypePin)
	{
		// Check linked pins
		if (RowTypePin->LinkedTo.IsEmpty() && !RowTypePin->DefaultObject)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT("NoRowType_Error", "GetAllRegistryRows requires a RowType to be specified. @@").ToString(), this);
			BreakAllNodeLinks();
			return;
		}
	}

	// Spawn intermediate UK2Node_CallFunction for UDalUtilsLibrary::K2_GetAllRegistryRows
	static const FName FunctionName = GET_FUNCTION_NAME_CHECKED(UDalUtilsLibrary, K2_GetAllRegistryRows);
	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallFunction->FunctionReference.SetExternalMember(FunctionName, UDalUtilsLibrary::StaticClass());
	CallFunction->AllocateDefaultPins();

	bool bIsErrorFree = true;

	// Connect RowType input: InRowStruct parameter
	{
		static const FName InRowStructParamName = TEXT("InRowStruct");
		UEdGraphPin* CallRowStructPin = CallFunction->FindPin(InRowStructParamName);
		if (RowTypePin && CallRowStructPin)
		{
			if (RowTypePin->LinkedTo.Num() > 0)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*RowTypePin, *CallRowStructPin).CanSafeConnect();
			}
			else
			{
				CallRowStructPin->DefaultObject = RowTypePin->DefaultObject;
			}
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	// Connect OutRows output: set the output array element type to match the selected struct
	{
		static const FName OutRowsParamName = TEXT("OutRows");
		UEdGraphPin* CallOutRowsPin = CallFunction->FindPin(OutRowsParamName);
		if (ResultPin && CallOutRowsPin)
		{
			CallOutRowsPin->PinType = ResultPin->PinType;
			CallOutRowsPin->PinType.PinSubCategoryObject = ResultPin->PinType.PinSubCategoryObject;
			bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*ResultPin, *CallOutRowsPin).CanSafeConnect();
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "K2Node_GetAllRegistryRows: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

// Protected helpers

UEdGraphPin* UK2Node_GetAllRegistryRows::GetRowTypePin() const
{
	UEdGraphPin* Pin = FindPin(GetAllRegistryRowsHelper::RowTypePinName);
	check(!Pin || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GetAllRegistryRows::GetResultPin() const
{
	UEdGraphPin* Pin = FindPin(GetAllRegistryRowsHelper::OutRowsPinName);
	check(!Pin || Pin->Direction == EGPD_Output);
	return Pin;
}

void UK2Node_GetAllRegistryRows::RefreshOutputPinType()
{
	UEdGraphPin* RowTypePin = GetRowTypePin();
	UEdGraphPin* ResultPin = GetResultPin();
	if (!RowTypePin || !ResultPin)
	{
		return;
	}

	const UScriptStruct* SelectedStruct = Cast<UScriptStruct>(RowTypePin->DefaultObject);
	const bool bHasValidStruct = SelectedStruct != nullptr;

	// Update the array element type
	ResultPin->PinType.PinCategory = bHasValidStruct ? UEdGraphSchema_K2::PC_Struct : UEdGraphSchema_K2::PC_Wildcard;
	ResultPin->PinType.PinSubCategoryObject = bHasValidStruct ? const_cast<UScriptStruct*>(SelectedStruct) : nullptr;
	ResultPin->PinType.ContainerType = EPinContainerType::Array;

	// Notify the graph about pin type change
	GetGraph()->NotifyNodeChanged(this);
}

#undef LOCTEXT_NAMESPACE
