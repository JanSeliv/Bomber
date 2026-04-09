// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Abilities/BmrPowerupCollectAbility.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPowerupActor.h"
#include "Components/BmrMapComponent.h"
#include "DataRegistries/BmrPowerupRow.h"
#include "Structures/BmrPowerupTag.h"

// UE
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupCollectAbility)

// Is overridden to prevent event-based activation if pickup is not allowed
bool UBmrPowerupCollectAbility::ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData) const
{
	if (!Super::ShouldAbilityRespondToEvent(ActorInfo, TriggerEventData))
	{
		return false;
	}

	// Make sure instigator is represented on the map (has MapComponent) to prevent picking up by non-gameplay actors, such as UI or cosmetic actors
	const AActor* Instigator = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	return UBmrMapComponent::GetMapComponent(Instigator) != nullptr;
}

// Actually activate ability, do not call this directly
void UBmrPowerupCollectAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	check(ActorInfo && TriggerEventData);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null!"), __LINE__, __FUNCTION__);
	const ABmrPowerupActor& PowerupActor = *CastChecked<ABmrPowerupActor>(TriggerEventData->Target);

	// Apply the collect gameplay effect to increase own attribute
	const FBmrPowerupTag PowerupTag = TriggerEventData->TargetTags.GetByIndex(0);
	const FBmrPowerupRow* PowerupRow = FBmrPowerupRow::GetRowByPowerupTag(PowerupTag);
	const TSubclassOf<UGameplayEffect> CollectGameplayEffect = PowerupRow ? PowerupRow->CollectGameplayEffect.Get() : nullptr;
	if (ensureMsgf(CollectGameplayEffect, TEXT("ASSERT: [%i] %hs:\n'CollectGameplayEffect' failed to obtain!"), __LINE__, __FUNCTION__))
	{
		FGameplayEffectContextHandle CollectContext = ASC->MakeEffectContext();
		CollectContext.AddSourceObject(TriggerEventData->Target);
		const FPredictionKey PredictionKey = ASC->GetPredictionKeyForNewAction();
		ASC->ApplyGameplayEffectToSelf(CollectGameplayEffect.GetDefaultObject(), GetAbilityLevel(), CollectContext, PredictionKey);
	}

	// @TODO JanSeliv uL3AzYIa - BEGIN: remove next once provided support for predicted destroy pooled actors
	if (!PowerupActor.HasAuthority())
	{
		const_cast<ABmrPowerupActor&>(PowerupActor).SetActorHiddenInGame(true);
	}
	// @TODO JanSeliv uL3AzYIa - END
	else
	{
		// Finally, destroy powerup actor at the end
		UBmrMapComponent* InstigatorMapComponent = UBmrMapComponent::GetMapComponent(&PowerupActor);
		ABmrGeneratedMap::Get().DestroyLevelActor(InstigatorMapComponent, ActorInfo->AvatarActor.Get());
	}

	K2_EndAbilityLocally();
}