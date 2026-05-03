// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Abilities/BmrPlayerDeathAbility.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Components/BmrMapComponent.h"
#include "DataRegistries/BmrPlayerRow.h"
#include "GameFramework/BmrPlayerState.h"

// UE
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerDeathAbility)

// Actually activate ability, do not call this directly
void UBmrPlayerDeathAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	check(ActorInfo && TriggerEventData);

	const ABmrPlayerState* MyPlayerState = Cast<ABmrPlayerState>(ActorInfo->OwnerActor.Get());
	const FBmrPlayerRow* PlayerRow = MyPlayerState ? FBmrPlayerRow::GetRowByPlayerTag(MyPlayerState->GetPlayerTag()) : nullptr;
	UAnimMontage* DeathMontage = PlayerRow ? PlayerRow->DeathMontage.Get() : nullptr;
	if (!ensureMsgf(DeathMontage, TEXT("ASSERT: [%i] %hs:\n'DeathMontage' failed to play!"), __LINE__, __FUNCTION__))
	{
		K2_EndAbilityLocally();
		return;
	}

	// Play death animation
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DeathMontage);
	MontageTask->ReadyForActivation();

	// Unregister from the level, so other players/AI and bombs will not react to this pawn anymore
	AActor* DeathCauser = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	ABmrGeneratedMap::Get().RemoveFromGrid(UBmrMapComponent::GetMapComponent(ActorInfo->AvatarActor.Get()), DeathCauser);
}
