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
			EndGameStateInternal = EEndGameState::None;
			OnEndGameStateChanged.Broadcast(EndGameStateInternal);
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
		EndGameStateInternal = EEndGameState::Draw;
		OnEndGameStateChanged.Broadcast(EndGameStateInternal);
		return;
	}

	// Game is running

	// locals
	bool bUpdateGameState = false;
	const EEndGameState CurrentEndGameState = EndGameStateInternal;
	const int32 PlayerNum = USingletonLibrary::GetAlivePlayersNum();
	const APawn* PawnOwner = GetPawn();
	if (!PawnOwner
	    || !PawnOwner->GetController()) // is dead owner
	{
		if (PlayerNum <= 0) // last players were blasted together
		{
			EndGameStateInternal = EEndGameState::Draw;
			bUpdateGameState = true; // no players to play, game ended
		}
		else
		{
			EndGameStateInternal = EEndGameState::Lose;
			bUpdateGameState = PlayerNum == 1;
		}
	}
	else if (PlayerNum == 1) // is alive owner and is the last player
	{
		EndGameStateInternal = EEndGameState::Win;
		bUpdateGameState = true; // we have winner, game ended
	}

	// Need to notify that the game was ended
	if (bUpdateGameState)
	{
		MyGameState->ServerSetGameState(ECGS::EndGame);
	}

	if (CurrentEndGameState != EndGameStateInternal
	    && OnEndGameStateChanged.IsBound())
	{
		OnEndGameStateChanged.Broadcast(EndGameStateInternal);
	}
}

// Is called on clients to apply current End-Game state
void AMyPlayerState::OnRep_EndGameState()
{
	OnEndGameStateChanged.Broadcast(EndGameStateInternal);
}
