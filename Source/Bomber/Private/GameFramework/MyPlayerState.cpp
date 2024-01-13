// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPlayerState)

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

// Called when the game starts
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Listen states
		if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
		{
			MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);

			// Handle current game state if initialized with delay
			if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
			{
				OnGameStateChanged(ECurrentGameState::Menu);
			}
		}
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

	const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
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
	const int32 PlayerNum = UMyBlueprintFunctionLibrary::GetAlivePlayersNum();

	if (!IsPlayerAlive())
	{
		if (PlayerNum <= 0) // last players were blasted together
		{
			MulticastSetEndGameState(EEndGameState::Draw);
		}
		else
		{
			MulticastSetEndGameState(EEndGameState::Lose);
		}
	}
	else if (PlayerNum == 1) // is alive owner and is the last player
	{
		MulticastSetEndGameState(EEndGameState::Win);
	}
}

// Returns true if current player is alive
bool AMyPlayerState::IsPlayerAlive() const
{
	const APawn* PawnOwner = GetPawn();
	return PawnOwner && PawnOwner->GetController() != nullptr;
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
