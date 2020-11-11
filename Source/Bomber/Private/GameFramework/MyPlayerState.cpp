// Copyright 2020 Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Globals/SingletonLibrary.h"
#include "GameFramework/MyGameStateBase.h"
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

//
void AMyPlayerState::ChoosePlayer(USkeletalMesh* MeshAsset)
{
	if (!MeshAsset)
	{
		return;
	}

	ChosenMeshInternal = MeshAsset;

	if (UMapComponent* MapComponent = UMapComponent::GetMapComponent(GetPawn()))
	{
		MapComponent->SetMesh(MeshAsset);
	}
}

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
			ServerUpdateEndState();
			break;
		}
		default: break;
	}
}

// Updated result of the game for controlled player after ending the game. Called when one of players is destroying
void AMyPlayerState::ServerUpdateEndState_Implementation()
{
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState(this);
	if (CurrentGameState == ECurrentGameState::None     // is not valid game state, nullptr or not fully initialized
	    || EndGameStateInternal != EEndGameState::None) // end state was set already for current game
	{
		return;
	}

	// handle timer is 0
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
	if (!IS_VALID(PawnOwner)) // is dead owner
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
		if (PawnOwner->GetController<APlayerController>()) // is player
		{
			EndGameStateInternal = EEndGameState::Win;
		}
		bUpdateGameState = true; // we have winner, game ended
	}

	// Need to notify that the game was ended
	if (bUpdateGameState)
	{
		USingletonLibrary::GetMyGameState(this)->ServerSetGameState(ECurrentGameState::EndGame);
	}
}
