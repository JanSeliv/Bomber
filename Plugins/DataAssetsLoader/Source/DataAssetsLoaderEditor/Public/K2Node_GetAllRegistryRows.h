// Copyright (c) Yevhenii Selivanov

#pragma once

#include "K2Node.h"

#include "K2Node_GetAllRegistryRows.generated.h"

/**
 * Blueprint node that retrieves all cached rows from a Data Registry.
 * Input: UScriptStruct* dropdown to select the row type.
 * Output: TArray of the selected struct type (dynamic typed output pin).
 */
UCLASS()
class DATAASSETSLOADEREDITOR_API UK2Node_GetAllRegistryRows : public UK2Node
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode interface

	//~ Begin UK2Node interface
	virtual bool IsNodePure() const override { return true; }
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node interface

protected:
	/** Returns the RowType input pin */
	UEdGraphPin* GetRowTypePin() const;

	/** Returns the output array pin */
	UEdGraphPin* GetResultPin() const;

	/** Updates the output pin type based on the selected struct */
	void RefreshOutputPinType();
};
