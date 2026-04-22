// Copyright (c) Yevhenii Selivanov

#pragma once

#include "K2Node.h"

#include "K2Node_ListenForDataRegistryRow.generated.h"

/**
 * Custom blueprint node that wraps UDalRegistrySubsystem::BPListenForDataRegistryRow with a latent-style interface:
 * accepts a row struct type + row name input and fires a Completed exec pin once the row and its soft references are resolvable
 */
UCLASS()
class DATAASSETSLOADEREDITOR_API UK2Node_ListenForDataRegistryRow : public UK2Node
{
	GENERATED_BODY()

public:
	static inline const FName ItemIdPinName = TEXT("ItemId");
	static inline const FName CompletedPinName = TEXT("Completed");

	/** Returns the name of the native function to call */
	static FName GetNativeFunctionName();

	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual bool IsNodePure() const override { return false; }
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FName GetCornerIcon() const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	// End of UK2Node interface
};