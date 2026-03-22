// Copyright (c) Yevhenii Selivanov

#pragma once

#include "K2Node.h"

#include "K2Node_ListenForDataAsset.generated.h"

/**
 * Custom blueprint node that wraps UDalSubsystem::BPListenForDataAsset with a latent-style interface:
 * accepts a data asset class input and fires a Completed exec pin with the loaded data asset as output
 */
UCLASS()
class DATAASSETSLOADEREDITOR_API UK2Node_ListenForDataAsset : public UK2Node
{
	GENERATED_BODY()

public:
	static inline const FName DataAssetClassPinName = TEXT("DataAssetClass");
	static inline const FName CompletedPinName = TEXT("Completed");
	static inline const FName FailedPinName = TEXT("Failed");
	static inline const FName DataAssetPinName = TEXT("DataAsset");

	/** Returns the name of the native function to call */
	static FName GetNativeFunctionName();

	/** Returns the selected data asset class from the class input pin, or nullptr if not set */
	UClass* GetSelectedClass(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;

	/** Updates the output pin type to match the selected class */
	void OnClassPinChanged() const;

	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual bool IsNodePure() const override { return false; }
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual FName GetCornerIcon() const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	// End of UK2Node interface
};
