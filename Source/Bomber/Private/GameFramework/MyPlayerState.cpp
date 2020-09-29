// Copyright 2020 Yevhenii Selivanov

#include "MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "MyGameStateBase.h"
#include "SingletonLibrary.h"
//---
#include "UnrealNetwork.h"

/* ---------------------------------------------------
 *		Protected
 * --------------------------------------------------- */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EndGameStateInternal);
}

// Called when the game starts. Created widget
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
		MyGameState->OnAnyPlayerDestroyed.AddDynamic(this, &ThisClass::ServerUpdateEndState);
	}
}

//
void AMyPlayerState::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			EndGameStateInternal = EEndGameState::None;
			break;
		}
		case ECurrentGameState::EndGame:
		{
			ServerUpdateEndState(GetPawn());
			break;
		}
		default: break;
	}
}

//
void AMyPlayerState::ServerUpdateEndState_Implementation(const APawn* Pawn)
{
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState(this);
	if (CurrentGameState == ECurrentGameState::None     // is not valid game state, nullptr or not fully initialized
	    || EndGameStateInternal != EEndGameState::None) // end state was set already for current game
	{
		return;
	}

	if (CurrentGameState == ECurrentGameState::EndGame) // game was finished
	{
		EndGameStateInternal = EEndGameState::Draw;
		return;
	}

	if (CurrentGameState != ECurrentGameState::InGame)
	{
		return;
	}
	// Game is running

	// locals
	bool bUpdateGameState = false;
	const int32 PlayerNum = USingletonLibrary::GetAlivePlayersNum();
	const APawn* PawnOwner = GetPawn();

	if (PawnOwner == Pawn) // is owner destroyed
	{
		if (!PlayerNum) // last players were blasted together
		{
			EndGameStateInternal = EEndGameState::Draw;
			bUpdateGameState = true; // no players to play, game ended
		}
		else if (!IS_VALID(Pawn)) // is dead
		{
			EndGameStateInternal = EEndGameState::Lose;
		}
	}
	else if (PlayerNum == 1) // another player was destroyed and is alive only 1 player
	{
		if (IS_VALID(PawnOwner)) // the owner is left
		{
			EndGameStateInternal = EEndGameState::Win;
			bUpdateGameState = true; // we have winner, game ended
		}
	}

	// Need to notify that the game was ended
	if (bUpdateGameState)
	{
		USingletonLibrary::GetMyGameState(this)->ServerSetGameState(ECurrentGameState::EndGame);
	}
}
