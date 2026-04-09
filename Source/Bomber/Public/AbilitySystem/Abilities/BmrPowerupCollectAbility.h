// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Abilities/GameplayAbility.h"

#include "BmrPowerupCollectAbility.generated.h"

/**
 * Is activated when player or AI collects a powerup powerup.
 * Ability is triggered by the TAG_EVENT_POWERUP_COLLECTED, where:
 * - EventData.Instigator: actor who collected the powerup (overlap causer).
 * - EventData.Target: powerup actor, which was collected.
 * - EventData.TargetTags: powerup actor tags, such as powerup type.
 */
UCLASS()
class BOMBER_API UBmrPowerupCollectAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	/** Is overridden to prevent event-based activation if pickup is not allowed. */
	virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData) const override;

	/** Actually activate ability, do not call this directly. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
