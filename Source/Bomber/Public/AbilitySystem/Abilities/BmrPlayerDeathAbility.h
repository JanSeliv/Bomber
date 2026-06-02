// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Abilities/GameplayAbility.h"

#include "BmrPlayerDeathAbility.generated.h"

/**
 * Handles player death state and cleanup when health reaches zero.
 * Ability is triggered by the BmrGameplayTags::Event::Player_Death event, where:
 * - EventData.Instigator: the actor that caused the death (bomb, environment, etc.).
 */
UCLASS()
class BOMBER_API UBmrPlayerDeathAbility : public UGameplayAbility
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Actually activate ability, do not call this directly. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called once death montage finished, blended out or was interrupted, ends the ability so its Activation Owned Tag no longer lingers across restart. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Bomber]")
	void OnDeathMontageFinished();
};
