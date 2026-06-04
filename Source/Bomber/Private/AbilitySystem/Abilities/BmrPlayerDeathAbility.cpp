// Copyright (c) Yevhenii Selivanov

#include "AbilitySystem/Abilities/BmrPlayerDeathAbility.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Components/BmrMapComponent.h"
#include "DalRegistrySubsystem.h"
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

	// Unregister from the level FIRST so the pawn is marked dead and removed, other players/AI and bombs will not react to this pawn anymore
	AActor* DeathCauser = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	ABmrGeneratedMap::Get().RemoveFromGrid(UBmrMapComponent::GetMapComponent(ActorInfo->AvatarActor.Get()), DeathCauser);

	const ABmrPlayerState* MyPlayerState = Cast<ABmrPlayerState>(ActorInfo->OwnerActor.Get());
	const FName RowName = MyPlayerState ? MyPlayerState->GetChosenMeshData().RowName : NAME_None;
	UDalRegistrySubsystem::Get().ListenForDataRegistryRow<FBmrPlayerRow>(this, RowName, &ThisClass::OnPlayerRowLoaded);
}

// Called once player row becomes resolvable in Data Registry, plays its death montage
void UBmrPlayerDeathAbility::OnPlayerRowLoaded_Implementation(const FBmrPlayerRow& PlayerRow)
{
	UAnimMontage* DeathMontage = PlayerRow.DeathMontage.Get();
	if (!ensureMsgf(DeathMontage, TEXT("ASSERT: [%i] %hs:\n'DeathMontage' failed to play!"), __LINE__, __FUNCTION__))
	{
		K2_EndAbilityLocally();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DeathMontage);
	checkf(MontageTask, TEXT("ERROR: [%i] %hs:\n'MontageTask' is null!"), __LINE__, __FUNCTION__);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
	MontageTask->ReadyForActivation();
}

// Called once death montage finished, blended out or was interrupted
void UBmrPlayerDeathAbility::OnDeathMontageFinished_Implementation()
{
	K2_EndAbility();
}
