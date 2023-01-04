// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "NewAIDataAsset.generated.h"

/**
 * Contains all assets in NewAI plugin
 */
UCLASS()
class NEWAIRUNTIME_API UNewAIDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Behaviour Tree
	 * @see UNewAIDataAsset::BehaviorTreeInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeInternal; }

protected:
	/** The Behaviour Tree that is used in AAIController for bots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Behavior Tree"))
	TObjectPtr<class UBehaviorTree> BehaviorTreeInternal = nullptr;
};
