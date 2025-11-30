// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Abilities/BmrPowerupCollectAbility.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPowerupActor.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrPowerupDataAsset.h"
#include "Structures/BmrPowerupTag.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupCollectAbility)

// Actually activate ability, do not call this directly
void UBmrPowerupCollectAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	check(ActorInfo && TriggerEventData);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	checkf(ASC, TEXT("ERROR: [%i] %hs:\n'ASC' is null!"), __LINE__, __FUNCTION__);
	const ABmrPowerupActor& ItemActor = *CastChecked<ABmrPowerupActor>(TriggerEventData->Instigator);

	// Apply the collect gameplay effect to increase own attribute
	const FBmrPowerupTag PowerupTag = TriggerEventData->InstigatorTags.GetByIndex(0);
	const UBmrPowerupRow* ItemRow = UBmrPowerupDataAsset::Get().GetRowByItemType(PowerupTag, UBmrBlueprintFunctionLibrary::GetLevelType());
	const TSubclassOf<UGameplayEffect> CollectGameplayEffect = ItemRow ? ItemRow->CollectGameplayEffect : nullptr;
	if (ensureMsgf(CollectGameplayEffect, TEXT("ASSERT: [%i] %hs:\n'CollectGameplayEffect' failed to obtain!"), __LINE__, __FUNCTION__))
	{
		FGameplayEffectContextHandle CollectContext = ASC->MakeEffectContext();
		CollectContext.AddSourceObject(TriggerEventData->Instigator);
		const FPredictionKey PredictionKey = ASC->GetPredictionKeyForNewAction();
		ASC->ApplyGameplayEffectToSelf(CollectGameplayEffect.GetDefaultObject(), GetAbilityLevel(), CollectContext, PredictionKey);
	}

	// @TODO JanSeliv uL3AzYIa - BEGIN: remove next once provided support for predicted destroy pooled actors
	if (!ItemActor.HasAuthority())
	{
		const_cast<ABmrPowerupActor&>(ItemActor).SetActorHiddenInGame(true);
	}
	// @TODO JanSeliv uL3AzYIa - END
	else
	{
		// Finally, destroy powerup actor at the end
		UBmrMapComponent* InstigatorMapComponent = UBmrMapComponent::GetMapComponent(&ItemActor);
		ABmrGeneratedMap::Get().DestroyLevelActor(InstigatorMapComponent, ActorInfo->AvatarActor.Get());
	}

	K2_EndAbilityLocally();
}