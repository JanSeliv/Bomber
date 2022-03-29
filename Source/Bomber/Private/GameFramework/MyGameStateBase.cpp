// Copyright 2021 Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "Controllers/MyPlayerController.h"
#include "Globals/SingletonLibrary.h"
//---
#include "Net/UnrealNetwork.h"

// Default constructor
AMyGameStateBase::AMyGameStateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

//  Returns the AMyGameStateBase::CurrentGameState property
ECurrentGameState AMyGameStateBase::GetCurrentGameState(const UObject* WorldContextObject)
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
}

// Called when the game starts or when spawned
void AMyGameStateBase::BeginPlay()
{
	Super::BeginPlay();
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
			ServerOnGameStarting();
			break;
		}
		case ECurrentGameState::EndGame:
			break;
		case ECurrentGameState::InGame:
		{
			ServerStartInGameCountdown();
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

// Called when game enters to the Game Starting state and timer starts countdown
void AMyGameStateBase::ServerOnGameStarting_Implementation()
{
	const UWorld* World = GetWorld();
	if (!World
	    || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::GameStarting, TEXT("ASSERT: Is not the Game Starting event on the server side")))
	{
		return;
	}

	// Clear timer
	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(StartingTimerInternal);

	// Decrement starting countdown timer
	TWeakObjectPtr<ThisClass> WeakThis(this);
	TimerManager.SetTimer(StartingTimerInternal, [WeakThis]
	{
		AMyGameStateBase* MyGameStateBase = WeakThis.Get();
		if (MyGameStateBase
		    && MyGameStateBase->CurrentGameStateInternal == ECurrentGameState::GameStarting) // state was not changed
		{
			MyGameStateBase->ServerSetGameState(ECurrentGameState::InGame);
		}
	}, StartingCountdownInternal, false);
}

// Decrement the countdown timer of the current game.
void AMyGameStateBase::ServerStartInGameCountdown_Implementation()
{
	const UWorld* World = GetWorld();
	if (!World
	    || InGameCountdownInternal <= 0
	    || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::InGame, TEXT("ASSERT: 'CurrentGameStateInternal == ECurrentGameState::InGame' condition is FALSE")))
	{
		return;
	}

	// Clear timer
	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(InGameTimerInternal);

	// Decrement in-game countdown timer
	TWeakObjectPtr<ThisClass> WeakThis(this);
	TimerManager.SetTimer(InGameTimerInternal, [WeakThis]
	{
		AMyGameStateBase* MyGameStateBase = WeakThis.Get();
		if (MyGameStateBase
		    && MyGameStateBase->CurrentGameStateInternal == ECurrentGameState::InGame) // state was not changed
		{
			MyGameStateBase->ServerSetGameState(ECurrentGameState::EndGame);
		}
	}, InGameCountdownInternal, false);
}
