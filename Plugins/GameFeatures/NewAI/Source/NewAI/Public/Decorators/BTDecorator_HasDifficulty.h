// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasDifficulty.generated.h"

/**
 * Decorator node that bases its condition on whether the game difficulty level is matched with one or more specified types.
 */
UCLASS(Blueprintable, BlueprintType)
class NEWAI_API UBTDecorator_HasDifficulty : public UBTDecorator
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBTDecorator_HasDifficulty();

	/** Returns true if the game difficulty level is matched with one or more specified types. */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** Returns string containing the description of the decorator's properties. */
	virtual FString GetStaticDescription() const override;

protected:
	/** Select one or more game difficulty levels to match. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "Difficulties", Bitmask, BitmaskEnum = "/Script/Bomber.EGameDifficulty"))
	int32 DifficultiesInternal = 0;
};
