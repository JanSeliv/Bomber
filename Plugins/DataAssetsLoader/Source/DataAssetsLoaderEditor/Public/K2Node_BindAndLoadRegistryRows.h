// Copyright (c) Yevhenii Selivanov

#pragma once

#include "K2Node.h"

#include "K2Node_BindAndLoadRegistryRows.generated.h"

/**
 * Custom blueprint node that wraps UDalRegistrySubsystem::BPBindAndLoad with a latent-style interface:
 * accepts a row struct type input and fires a Completed exec pin when rows change and soft references finish loading
 */
UCLASS()
class DATAASSETSLOADEREDITOR_API UK2Node_BindAndLoadRegistryRows : public UK2Node
{
	GENERATED_BODY()

public:
	static inline const FName RowStructPinName = TEXT("RowStruct");
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
