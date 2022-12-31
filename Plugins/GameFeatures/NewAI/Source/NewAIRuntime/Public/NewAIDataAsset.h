// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/DataAsset.h"
#include "NewAIDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class NEWAIRUNTIME_API UNewAIDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Behaviour Tree
	 * @see UNewAIDataAsset::BehaviorTreeInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeInternal; }

protected:
	/** The Behaviour Tree that is used in AAIController for bots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "C++",
		meta = (BlueprintProtected, DisplayName = "Behaviour Tree asset"))
	TObjectPtr<UBehaviorTree> BehaviorTreeInternal;
};
