// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Abilities/BmrPowerupCollectAbility.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPowerupActor.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrPlayerDataAsset.h"
#include "DataRegistries/BmrPowerupRow.h"
#include "Structures/BmrPowerupTag.h"

// UE
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupCollectAbility)

// Default constructor
UBmrPowerupCollectAbility::UBmrPowerupCollectAbility()
{
	// Next pickup retriggers instance while its montage is still playing
	bRetriggerInstancedAbility = true;
}

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

	UAnimMontage* PickupMontage = UBmrPlayerDataAsset::Get().GetPickupMontage();
	if (!ensureMsgf(PickupMontage, TEXT("ASSERT: [%i] %hs:\n'PickupMontage' is null!"), __LINE__, __FUNCTION__))
	{
		K2_EndAbilityLocally();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, PickupMontage);
	checkf(MontageTask, TEXT("ERROR: [%i] %hs:\n'MontageTask' is null!"), __LINE__, __FUNCTION__);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnPickupMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnPickupMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnPickupMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnPickupMontageFinished);
	MontageTask->ReadyForActivation();
	ASC->ForceReplication();
}

// Called when pickup montage finishes, blends out or is interrupted
void UBmrPowerupCollectAbility::OnPickupMontageFinished_Implementation()
{
	K2_EndAbilityLocally();
}