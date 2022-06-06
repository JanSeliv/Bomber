// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
//---
#include "Net/UnrealNetwork.h"

/* ---------------------------------------------------
 *		Protected
 * --------------------------------------------------- */

AMyPlayerState::AMyPlayerState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Set the custom player name by user input
void AMyPlayerState::SetPlayerNameCustom(FName NewName)
{
	if (CustomPlayerNameInternal == NewName
	    || NewName.IsNone())
	{
		return;
	}

	bUseCustomPlayerNames = true;
	CustomPlayerNameInternal = NewName;

	SetPlayerName(NewName.ToString());

	if (OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(NewName);
	}
}

// Returns custom player name
FName AMyPlayerState::GetPlayerFNameCustom() const
{
	if (CustomPlayerNameInternal.IsNone())
	{
		// Return default name
		const FName PlatformUserName(UKismetSystemLibrary::GetPlatformUserName());
		return PlatformUserName;
	}

	return CustomPlayerNameInternal;
}

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EndGameStateInternal);
	DOREPLIFETIME(ThisClass, CustomPlayerNameInternal);
}

// Called when the game starts. Created widget
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Listen game states to notify server about ending game for controlled player
void AMyPlayerState::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			MulticastSetEndGameState(EEndGameState::None);
			break;
		}
		case ECurrentGameState::EndGame:
		{
			UpdateEndGameState();
			break;
		}
		default:
			break;
	}
}

// Updates result of the game for controlled player
void AMyPlayerState::UpdateEndGameState()
{
	if (!HasAuthority())
	{
		return;
	}

	AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState();
	const ECurrentGameState CurrentGameState = MyGameState ? MyGameState->GetCurrentGameState() : ECGS::None;
	if (CurrentGameState == ECGS::None                  // is not valid game state, nullptr or not fully initialized
	    || EndGameStateInternal != EEndGameState::None) // end state was set already for current game
	{
		return;
	}

	// handle timer is 0
	if (MyGameState->IsInGameTimerElapsed())
	{
		MulticastSetEndGameState(EEndGameState::Draw);
		return;
	}

	// Game is running

	// locals
	bool bUpdateGameState = false;
	const int32 PlayerNum = USingletonLibrary::GetAlivePlayersNum();
	const APawn* PawnOwner = GetPawn();
	if (!PawnOwner
	    || !PawnOwner->GetController()) // is dead owner
	{
		if (PlayerNum <= 0) // last players were blasted together
		{
			MulticastSetEndGameState(EEndGameState::Draw);
			bUpdateGameState = true; // no players to play, game ended
		}
		else
		{
			MulticastSetEndGameState(EEndGameState::Lose);
			bUpdateGameState = PlayerNum == 1;
		}
	}
	else if (PlayerNum == 1) // is alive owner and is the last player
	{
		MulticastSetEndGameState(EEndGameState::Win);
		bUpdateGameState = true; // we have winner, game ended
	}

	// Need to notify that the game was ended
	if (bUpdateGameState)
	{
		MyGameState->ServerSetGameState(ECGS::EndGame);
	}
}

// Set new End-Game state, is made as multicast to notify own client asap
void AMyPlayerState::MulticastSetEndGameState_Implementation(EEndGameState NewEndGameState)
{
	if (NewEndGameState == EndGameStateInternal)
	{
		return;
	}

	EndGameStateInternal = NewEndGameState;

	if (OnEndGameStateChanged.IsBound())
	{
		OnEndGameStateChanged.Broadcast(EndGameStateInternal);
	}
}
