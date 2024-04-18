// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPlayerState)

AMyPlayerState::AMyPlayerState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Makes GetPlayerName() to call virtual GetPlayerNameCustom() to allow custom access
	bUseCustomPlayerNames = true;
}

// Returns true if this Player State is controlled by a locally controlled player
bool AMyPlayerState::IsPlayerStateLocallyControlled() const
{
	const APlayerController* PC = GetPlayerController();
	return PC && PC->IsLocalPlayerController();
}

/*********************************************************************************************
 * End Game State
 ********************************************************************************************* */

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

	if (IsCharacterDead())
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
		OnEndGameStateChanged.Broadcast(NewEndGameState);
	}
}

/*********************************************************************************************
 * Player Name
 ********************************************************************************************* */

// Set the custom player name by user input
void AMyPlayerState::SetPlayerNameCustom(FName NewName)
{
	if (GetOldPlayerName() == NewName
	    || NewName.IsNone())
	{
		return;
	}

	// First set locally the old name while name is changing.
	// Once settings are saved, ApplyCustomPlayerName() will be called
	SetOldPlayerName(NewName.ToString());
}

// Returns custom player name
FName AMyPlayerState::GetPlayerFNameCustom() const
{
	const FString LocalNickname = GetOldPlayerName();
	if (!LocalNickname.IsEmpty())
	{
		return *LocalNickname;
	}

	if (!CustomPlayerNameInternal.IsNone())
	{
		return CustomPlayerNameInternal;
	}

	return *UKismetSystemLibrary::GetPlatformUserName();
}

// Called on server when settings are saved to apply new player name
void AMyPlayerState::ServerSetPlayerNameCustom_Implementation(FName NewName)
{
	SetPlayerNameCustom(NewName);
	ApplyCustomPlayerName();
}

// Applies and broadcasts player nam
void AMyPlayerState::ApplyCustomPlayerName()
{
	CustomPlayerNameInternal = GetPlayerFNameCustom();

	if (OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(CustomPlayerNameInternal);
	}
}

// Called on client when custom player name is changed
void AMyPlayerState::OnRep_CustomPlayerName()
{
	ApplyCustomPlayerName();
}

/*********************************************************************************************
 * Is Character Dead
 ********************************************************************************************* */

// Called when character dead status is changed: character was killed or revived
void AMyPlayerState::SetCharacterDead(bool bIsDead)
{
	if (bIsCharacterDeadInternal == bIsDead)
	{
		return;
	}

	bIsCharacterDeadInternal = bIsDead;
	ApplyIsCharacterDead();
}

// Called on client when character Dead status is changed
void AMyPlayerState::OnRep_IsCharacterDead()
{
	ApplyIsCharacterDead();
}

// Applies and broadcasts Is Character Dead status
void AMyPlayerState::ApplyIsCharacterDead()
{
	if (bIsCharacterDeadInternal)
	{
		UpdateEndGameState();
	}

	if (OnCharacterDeadChanged.IsBound())
	{
		OnCharacterDeadChanged.Broadcast(bIsCharacterDeadInternal);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EndGameStateInternal);
	DOREPLIFETIME(ThisClass, CustomPlayerNameInternal);
	DOREPLIFETIME(ThisClass, bIsCharacterDeadInternal);
}

// Called when the game starts
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
	}

	if (IsPlayerStateLocallyControlled())
	{
		// Listen game settings to apply them once saved
		UMyGameUserSettings::Get().OnSaveSettings.AddUniqueDynamic(this, &ThisClass::OnSaveSettings);

		// Apply custom player name from config if any
		if (!CustomPlayerNameInternal.IsNone())
		{
			SetPlayerNameCustom(CustomPlayerNameInternal);
			ApplyCustomPlayerName();
		}
	}
}

// Listens game settings to apply them once saved
void AMyPlayerState::OnSaveSettings_Implementation()
{
	const FName LocalNickname = GetPlayerFNameCustom();
	if (CustomPlayerNameInternal != LocalNickname)
	{
		// Apply local player name on server
		ServerSetPlayerNameCustom(LocalNickname);
		ApplyCustomPlayerName();
	}
}

// Listen game states to notify server about ending game for controlled player
void AMyPlayerState::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
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

	if (CurrentGameState != ECGS::EndGame)
	{
		SetCharacterDead(false);
	}
}