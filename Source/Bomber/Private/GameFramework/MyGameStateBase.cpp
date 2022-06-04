// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "Controllers/MyPlayerController.h"
#include "Globals/DataAssetsContainer.h"
#include "Globals/SingletonLibrary.h"
//---
#include "Net/UnrealNetwork.h"

// Returns the Game State data asset
const UGameStateDataAsset& UGameStateDataAsset::Get()
{
	const UGameStateDataAsset* GameStateDataAsset = Cast<UGameStateDataAsset>(UDataAssetsContainer::GetGameStateDataAsset());
	checkf(GameStateDataAsset, TEXT("The Game State Data Asset is not valid"));
	return *GameStateDataAsset;
}

// Default constructor
AMyGameStateBase::AMyGameStateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Returns the AMyGameStateBase::CurrentGameState property
ECurrentGameState AMyGameStateBase::GetCurrentGameState()
{
	if (const AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		return MyGameState->CurrentGameStateInternal;
	}
	return ECurrentGameState::None;
}

// Returns the AMyGameState::CurrentGameState property.
void AMyGameStateBase::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	if (NewGameState == CurrentGameStateInternal)
	{
		return;
	}

	CurrentGameStateInternal = NewGameState;
	ApplyGameState();
}

/* ---------------------------------------------------
 *		Protected
 * --------------------------------------------------- */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameStateInternal);
	DOREPLIFETIME(ThisClass, StartingTimerSecRemainInternal);
	DOREPLIFETIME(ThisClass, InGameTimerSecRemainInternal);
}

// Updates current game state
void AMyGameStateBase::ApplyGameState()
{
	switch (CurrentGameStateInternal)
	{
		case ECurrentGameState::Menu:
			break;
		case ECurrentGameState::GameStarting:
		{
			StartingTimerSecRemainInternal = UGameStateDataAsset::Get().GetStartingCountdown();
			InGameTimerSecRemainInternal = UGameStateDataAsset::Get().GetInGameCountdown();
			TriggerStartingCountdown();
			break;
		}
		case ECurrentGameState::EndGame:
			break;
		case ECurrentGameState::InGame:
		{
			TriggerInGameCountdown();
			break;
		}
		default:
			break;
	}

	// Notify listeners
	if (OnGameStateChanged.IsBound())
	{
		OnGameStateChanged.Broadcast(CurrentGameStateInternal);
	}
}

// Called on the AMyGameState::CurrentGameState property updating.
void AMyGameStateBase::OnRep_CurrentGameState()
{
	ApplyGameState();
}

// Called when game enters to the Game Starting state to trigger its timer
void AMyGameStateBase::TriggerStartingCountdown()
{
	const UWorld* World = GetWorld();
	if (!World
	    || !HasAuthority()
	    || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::GameStarting, TEXT("ASSERT: Is not the Game Starting event on the server side")))
	{
		return;
	}

	// Decrement starting countdown timer
	constexpr bool bInLoop = true;
	constexpr float InRate = 1.f;
	World->GetTimerManager().SetTimer(StartingTimerInternal, this, &ThisClass::OnStartingTimerSecondDecremented, InRate, bInLoop);
}

// Is called each second during the Game Starting state for the 'Three-two-one-GO' timer
void AMyGameStateBase::OnStartingTimerSecondDecremented()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CurrentGameStateInternal != ECurrentGameState::GameStarting)
	{
		World->GetTimerManager().ClearTimer(StartingTimerInternal);
		return;
	}

	--StartingTimerSecRemainInternal;

	if (StartingTimerSecRemainInternal <= 0)
	{
		World->GetTimerManager().ClearTimer(StartingTimerInternal);
		ServerSetGameState(ECurrentGameState::InGame);
	}
}

// Called when game enters to the In-Game state to trigger its timer
void AMyGameStateBase::TriggerInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World
	    || !HasAuthority()
	    || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::InGame, TEXT("ASSERT: 'CurrentGameStateInternal == ECurrentGameState::InGame' condition is FALSE")))
	{
		return;
	}

	// Decrement in-game countdown timer
	constexpr bool bInLoop = true;
	constexpr float InRate = 1.f;
	World->GetTimerManager().SetTimer(InGameTimerInternal, this, &ThisClass::OnInGameTimerSecondDecremented, InRate, bInLoop);
}

// Is called each second during the In-Game state
void AMyGameStateBase::OnInGameTimerSecondDecremented()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CurrentGameStateInternal != ECurrentGameState::InGame)
	{
		World->GetTimerManager().ClearTimer(InGameTimerInternal);
		return;
	}

	--InGameTimerSecRemainInternal;

	if (InGameTimerSecRemainInternal <= 0)
	{
		World->GetTimerManager().ClearTimer(InGameTimerInternal);
		ServerSetGameState(ECurrentGameState::EndGame);
	}
}
