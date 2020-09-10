// Copyright 2020 Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "UnrealNetwork.h"

// Default constructor
AMyGameStateBase::AMyGameStateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Returns the AMyGameState::CurrentGameState property.
void AMyGameStateBase::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	CurrentGameStateInternal = NewGameState;
	OnRep_CurrentGameState();
}

bool AMyGameStateBase::ServerSetGameState_Validate(ECurrentGameState NewGameState)
{
	return true;
}

/* ---------------------------------------------------
*		Protected functions
* --------------------------------------------------- */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InGameCountdownInternal);
	DOREPLIFETIME(ThisClass, CurrentGameStateInternal);
}

// Called on the AMyGameState::CurrentGameState property updating.
void AMyGameStateBase::OnRep_CurrentGameState()
{
	switch (CurrentGameStateInternal)
	{
		case ECurrentGameState::Menu:
			break;
		case ECurrentGameState::GameStarting:
			OnGameStarting();
			break;
		case ECurrentGameState::InGame:
			ServerStartInGameCountdown();
			break;
		default: break;
	}

	// Notify listeners
	OnGameStateChanged.Broadcast(CurrentGameStateInternal);
}

//
void AMyGameStateBase::OnGameStarting()
{
	UWorld* World = GetWorld();
	if (!World
        || StartingCountdownInternal <= 0
        || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::GameStarting, TEXT("ASSERT: 'CurrentGameStateInternal == ECurrentGameState::InGame' condition is FALSE")))
	{
		return;
	}

	// Decrement in-game countdown timer
	TWeakObjectPtr<ThisClass> WeakThis(this);
	FTimerHandle InGameCountdownTimer;
	World->GetTimerManager().SetTimer(InGameCountdownTimer, [WeakThis]
    {
        if (AMyGameStateBase* MyGameStateBase = WeakThis.Get())
        {
            MyGameStateBase->ServerSetGameState(ECurrentGameState::InGame);
        }
    }, StartingCountdownInternal, false);
}

// Decrement the countdown timer of the current game.
void AMyGameStateBase::ServerStartInGameCountdown_Implementation()
{
	UWorld* World = GetWorld();
	if (!World
	    || InGameCountdownInternal <= 0
	    || !ensureMsgf(CurrentGameStateInternal == ECurrentGameState::InGame, TEXT("ASSERT: 'CurrentGameStateInternal == ECurrentGameState::InGame' condition is FALSE")))
	{
		return;
	}

	// Decrement in-game countdown timer
	TWeakObjectPtr<ThisClass> WeakThis(this);
	FTimerHandle InGameCountdownTimer;
	World->GetTimerManager().SetTimer(InGameCountdownTimer, [WeakThis]
	{
		if (AMyGameStateBase* MyGameStateBase = WeakThis.Get())
		{
			MyGameStateBase->InGameCountdownInternal--;
		}
	}, 1.F, true);
}

bool AMyGameStateBase::ServerStartInGameCountdown_Validate()
{
	return true;
}
