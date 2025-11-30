// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Abilities/GameplayAbility.h"

#include "BmrBombPlaceAbility.generated.h"

/**
 * Handles placing a bomb on the map for both player and AI characters.
 * Ability is triggered by the BmrGameplayTags::Event::Bomb_Place event, where:
 * - EventData.EventMagnitude: cell index to place the bomb at.
 */
UCLASS()
class BOMBER_API UBmrBombPlaceAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	/** Is overridden to prevent event-based activation if we bomb cannot be placed at the specified cell. */
	virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData) const override;

	/** Actually activate ability, do not call this directly. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Is overridden to apply cost with set by caller tag for bomb duration. */
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** Applies given gameplay effect with bomb duration. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	static FActiveGameplayEffectHandle ApplyDurationalEffect(TSubclassOf<UGameplayEffect> GameplayEffect, const FGameplayEffectContextHandle& EffectContext, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);
};
