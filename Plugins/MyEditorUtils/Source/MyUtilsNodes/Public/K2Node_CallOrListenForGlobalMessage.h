// Copyright (c) Yevhenii Selivanov

#pragma once

#include "K2Node.h"

#include "K2Node_CallOrListenForGlobalMessage.generated.h"

/**
 * Custom blueprint node that wraps UGlobalMessageSubsystem::BPCallOrStartListeningForGlobalMessage with a latent-style interface:
 * accepts a gameplay tag input and fires OnMessageReceived exec pin with the gameplay event payload as output
 */
UCLASS()
class MYUTILSNODES_API UK2Node_CallOrListenForGlobalMessage : public UK2Node
{
	GENERATED_BODY()

public:
	static inline const FName MessageTagPinName = TEXT("MessageTag");
	static inline const FName CompletedPinName = TEXT("Completed");
	static inline const FName OnMessageReceivedPinName = TEXT("OnMessageReceived");
	static inline const FName PayloadPinName = TEXT("MessagePayload");

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