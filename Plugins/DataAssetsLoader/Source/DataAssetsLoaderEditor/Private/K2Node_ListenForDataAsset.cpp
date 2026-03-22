// Copyright (c) Yevhenii Selivanov

#include "K2Node_ListenForDataAsset.h"

// DAL
#include "DalPrimaryDataAsset.h"
#include "DalSubsystem.h"

// UE
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_ListenForDataAsset)

#define LOCTEXT_NAMESPACE "K2Node_ListenForDataAsset"

// Returns the name of the native function to call
FName UK2Node_ListenForDataAsset::GetNativeFunctionName()
{
	return GET_FUNCTION_NAME_CHECKED(UDalSubsystem, BPListenForDataAsset);
}

// Returns the selected data asset class from the class input pin, or nullptr if not set
UClass* UK2Node_ListenForDataAsset::GetSelectedClass(const TArray<UEdGraphPin*>* InPinsToSearch /*= nullptr*/) const
{
	const TArray<UEdGraphPin*>& PinsToSearch = InPinsToSearch ? *InPinsToSearch : Pins;

	const UEdGraphPin* ClassPin = nullptr;
	for (const UEdGraphPin* PinIt : PinsToSearch)
	{
		if (PinIt->PinName == DataAssetClassPinName)
		{
			ClassPin = PinIt;
			break;
		}
	}

	if (!ClassPin)
	{
		return nullptr;
	}

	if (ClassPin->DefaultObject && ClassPin->LinkedTo.IsEmpty())
	{
		return CastChecked<UClass>(ClassPin->DefaultObject);
	}

	if (!ClassPin->LinkedTo.IsEmpty())
	{
		const UEdGraphPin* ClassSource = ClassPin->LinkedTo[0];
		return ClassSource ? Cast<UClass>(ClassSource->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return nullptr;
}

// Updates the output pin type to match the selected class
void UK2Node_ListenForDataAsset::OnClassPinChanged() const
{
	UEdGraphPin* DataAssetPin = FindPin(DataAssetPinName);
	if (!DataAssetPin)
	{
		return;
	}

	TArray<UEdGraphPin*> ConnectionList = DataAssetPin->LinkedTo;
	DataAssetPin->BreakAllPinLinks(true);

	const UClass* SelectedClass = GetSelectedClass();
	DataAssetPin->PinType.PinSubCategoryObject = SelectedClass
	                                                 ? const_cast<UClass*>(SelectedClass->GetAuthoritativeClass())
	                                                 : UDalPrimaryDataAsset::StaticClass();

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	for (UEdGraphPin* Connection : ConnectionList)
	{
		K2Schema->TryCreateConnection(DataAssetPin, Connection);
	}

	GetGraph()->NotifyGraphChanged();
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

// UEdGraphNode interface

void UK2Node_ListenForDataAsset::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	UEdGraphPin* CompletedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Completed);
	CompletedPin->PinToolTip = LOCTEXT("CompletedTooltip", "Called when the data asset is loaded and valid").ToString();

	UEdGraphPin* FailedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, FailedPinName);
	FailedPin->PinToolTip = LOCTEXT("FailedTooltip", "Called when the callback returned null, e.g. the data asset class is not registered or was unloaded").ToString();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, UDalPrimaryDataAsset::StaticClass(), DataAssetClassPinName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UDalPrimaryDataAsset::StaticClass(), DataAssetPinName);
}

void UK2Node_ListenForDataAsset::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin && ChangedPin->PinName == DataAssetClassPinName)
	{
		OnClassPinChanged();
	}
}

void UK2Node_ListenForDataAsset::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	if (Pin && Pin->PinName == DataAssetClassPinName)
	{
		OnClassPinChanged();
	}
}

FString UK2Node_ListenForDataAsset::GetPinMetaData(FName InPinName, FName InKey)
{
	if (InPinName == DataAssetClassPinName && InKey == TEXT("AllowAbstract"))
	{
		return TEXT("false");
	}

	return Super::GetPinMetaData(InPinName, InKey);
}

FText UK2Node_ListenForDataAsset::GetTooltipText() const
{
	const FName FunctionName = GetNativeFunctionName();
	const UFunction* Function = UDalSubsystem::StaticClass()->FindFunctionByName(FunctionName);
	return Function ? Function->GetToolTipText() : Super::GetTooltipText();
}

FText UK2Node_ListenForDataAsset::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Listen For Data Asset");
}

bool UK2Node_ListenForDataAsset::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	const EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	const bool bIsCompatible = GraphType == GT_Ubergraph || GraphType == GT_Macro;
	return bIsCompatible && Super::IsCompatibleWithGraph(TargetGraph);
}

// UK2Node interface

void UK2Node_ListenForDataAsset::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(Schema);
	bool bIsErrorFree = true;

	// Sequence node, defaults to two output pins
	UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();

	// Connect to input exe
	{
		UEdGraphPin* InputExePin = GetExecPin();
		UEdGraphPin* SequenceInputExePin = SequenceNode->GetExecPin();
		bIsErrorFree &= InputExePin && SequenceInputExePin && CompilerContext.MovePinLinksToIntermediate(*InputExePin, *SequenceInputExePin).CanSafeConnect();
	}

	// Create BPListenForDataAsset function call
	UK2Node_CallFunction* CallListenNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallListenNode->FunctionReference.SetExternalMember(GetNativeFunctionName(), UDalSubsystem::StaticClass());
	CallListenNode->AllocateDefaultPins();

	// Connect call to first sequence pin
	{
		UEdGraphPin* CallFunctionInputExePin = CallListenNode->GetExecPin();
		UEdGraphPin* SequenceFirstExePin = SequenceNode->GetThenPinGivenIndex(0);
		bIsErrorFree &= SequenceFirstExePin && CallFunctionInputExePin && Schema->TryCreateConnection(CallFunctionInputExePin, SequenceFirstExePin);
	}

	// Create local variable for data asset output, using base class to match delegate parameter type
	UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(this, UEdGraphSchema_K2::PC_Object, NAME_None, UDalPrimaryDataAsset::StaticClass());

	// Create assign node
	UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
	AssignNode->AllocateDefaultPins();

	UEdGraphPin* LoadedObjectVariablePin = TempVarOutput->GetVariablePin();

	// Connect local variable to assign node
	{
		UEdGraphPin* AssignLHSPin = AssignNode->GetVariablePin();
		bIsErrorFree &= AssignLHSPin && LoadedObjectVariablePin && Schema->TryCreateConnection(AssignLHSPin, LoadedObjectVariablePin);
	}

	// Connect local variable to output
	{
		UEdGraphPin* OutputDataAssetPin = FindPin(DataAssetPinName);
		bIsErrorFree &= LoadedObjectVariablePin && OutputDataAssetPin && CompilerContext.MovePinLinksToIntermediate(*OutputDataAssetPin, *LoadedObjectVariablePin).CanSafeConnect();
	}

	// Connect to DataAssetClass input
	{
		UEdGraphPin* CallClassPin = CallListenNode->FindPin(DataAssetClassPinName);
		UEdGraphPin* ClassPin = FindPin(DataAssetClassPinName);
		if (ClassPin && CallClassPin)
		{
			if (ClassPin->LinkedTo.Num() > 0)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*ClassPin, *CallClassPin).CanSafeConnect();
			}
			else
			{
				CallClassPin->DefaultValue = ClassPin->DefaultValue;
				CallClassPin->DefaultObject = ClassPin->DefaultObject;
			}
		}
		else
		{
			bIsErrorFree = false;
		}
	}

	// Create Completed delegate parameter
	UK2Node_CustomEvent* CompletedEventNode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	CompletedEventNode->CustomFunctionName = *FString::Printf(TEXT("%s_%s"), *CompletedPinName.ToString(), *CompilerContext.GetGuid(this));
	CompletedEventNode->AllocateDefaultPins();
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

	// Connect delegate
	{
		UEdGraphPin* CallFunctionDelegatePin = CallListenNode->FindPin(CompletedPinName);
		UEdGraphPin* EventDelegatePin = CompletedEventNode->FindPin(UK2Node_CustomEvent::DelegateOutputName);
		bIsErrorFree &= CallFunctionDelegatePin && EventDelegatePin && Schema->TryCreateConnection(CallFunctionDelegatePin, EventDelegatePin);
	}

	// Connect loaded data asset from event to assign
	{
		UEdGraphPin* LoadedAssetEventPin = CompletedEventNode->FindPin(DataAssetPinName);
		UEdGraphPin* AssignRHSPin = AssignNode->GetValuePin();
		bIsErrorFree &= AssignRHSPin && LoadedAssetEventPin && Schema->TryCreateConnection(LoadedAssetEventPin, AssignRHSPin);
	}

	// Connect assign exec input to event output
	{
		UEdGraphPin* CompletedEventOutputPin = CompletedEventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* AssignInputExePin = AssignNode->GetExecPin();
		bIsErrorFree &= AssignInputExePin && CompletedEventOutputPin && Schema->TryCreateConnection(AssignInputExePin, CompletedEventOutputPin);
	}

	// Branch on result validity: Completed if valid, Failed otherwise
	UK2Node_CallFunction* IsValidNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	IsValidNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid), UKismetSystemLibrary::StaticClass());
	IsValidNode->AllocateDefaultPins();
	bIsErrorFree &= Schema->TryCreateConnection(LoadedObjectVariablePin, IsValidNode->FindPinChecked(TEXT("Object")));

	UK2Node_IfThenElse* BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	BranchNode->AllocateDefaultPins();

	bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetThenPin(), BranchNode->GetExecPin());
	bIsErrorFree &= Schema->TryCreateConnection(IsValidNode->GetReturnValuePin(), BranchNode->GetConditionPin());

	{
		UEdGraphPin* OutputCompletedPin = FindPin(UEdGraphSchema_K2::PN_Completed);
		bIsErrorFree &= OutputCompletedPin && CompilerContext.MovePinLinksToIntermediate(*OutputCompletedPin, *BranchNode->GetThenPin()).CanSafeConnect();
	}

	{
		UEdGraphPin* OutputFailedPin = FindPin(FailedPinName);
		bIsErrorFree &= OutputFailedPin && CompilerContext.MovePinLinksToIntermediate(*OutputFailedPin, *BranchNode->GetElsePin()).CanSafeConnect();
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "K2Node_ListenForDataAsset: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

void UK2Node_ListenForDataAsset::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();

	const UClass* SelectedClass = GetSelectedClass(&OldPins);
	if (SelectedClass)
	{
		UEdGraphPin* DataAssetPin = FindPin(DataAssetPinName);
		if (DataAssetPin)
		{
			DataAssetPin->PinType.PinSubCategoryObject = const_cast<UClass*>(SelectedClass->GetAuthoritativeClass());
		}
	}
}

FName UK2Node_ListenForDataAsset::GetCornerIcon() const
{
	return TEXT("Graph.Latent.LatentIcon");
}

void UK2Node_ListenForDataAsset::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_ListenForDataAsset::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Data Assets Loader");
}

#undef LOCTEXT_NAMESPACE
