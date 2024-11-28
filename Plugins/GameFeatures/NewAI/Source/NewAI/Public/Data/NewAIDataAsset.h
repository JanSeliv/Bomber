// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"
//---
#include "NewAIDataAsset.generated.h"

/**
 * Contains all assets in NewAI plugin
 */
UCLASS()
class NEWAI_API UNewAIDataAsset : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * General
	 ********************************************************************************************* */
public:
	/** Returns this Data Asset, is checked and wil crash if can't be obtained, e.g: when is not set. */
	static const UNewAIDataAsset& Get(const UObject* OptionalWorldContext = nullptr);

	/** Returns the Behaviour Tree
	 * @see UNewAIDataAsset::BehaviorTreeInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeInternal; }

protected:
	/** The Behaviour Tree that is used in AAIController for bots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Behavior Tree"))
	TObjectPtr<class UBehaviorTree> BehaviorTreeInternal = nullptr;
};
