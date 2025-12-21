// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Abilities/GameplayAbility.h"

#include "BmrPowerupCollectAbility.generated.h"

/**
 * Is activated when player or AI collects a powerup powerup.
 * Ability is triggered by the TAG_EVENT_POWERUP_COLLECTED, where:
 * - EventData.Instigator: powerup actor, which was collected.
 */
UCLASS()
class BOMBER_API UBmrPowerupCollectAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	/** Actually activate ability, do not call this directly. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
