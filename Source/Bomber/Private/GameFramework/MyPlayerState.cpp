// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyGameUserSettings.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPlayerState)

AMyPlayerState::AMyPlayerState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Reset default value to -1 to avoid conflicts with first player of 0 ID
	SetPlayerId(INDEX_NONE);
}

// Returns true if this Player State is controlled by a locally controlled player
bool AMyPlayerState::IsPlayerStateLocallyControlled() const
{
	const APlayerController* PC = GetPlayerController();
	return PC && PC->IsLocalPlayerController();
}

// Returns always valid owner (human or bot), or crash if nullptr
APlayerCharacter& AMyPlayerState::GetPlayerCharacterChecked() const
{
	return *CastChecked<APlayerCharacter>(GetPawn());
}

/*********************************************************************************************
 * End Game State
 ********************************************************************************************* */

// Tries to set new End-Game state for this player
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
		SetEndGameState(EEndGameState::Draw);
		return;
	}

	// Game is running

	const EEndGameState NewEndGameState = [&]
	{
		const int32 PlayerNum = UMyBlueprintFunctionLibrary::GetAlivePlayersNum();

		if (IsCharacterDead())
		{
			// Draw: last players were blasted together
			return PlayerNum <= 0 ? EEndGameState::Draw : EEndGameState::Lose;
		}

		// Win: Is alive owner and is the last player
		return PlayerNum == 1 ? EEndGameState::Win : EEndGameState::None;
	}();

	SetEndGameState(NewEndGameState);
}

// Sets End-Game state to the specified one
void AMyPlayerState::SetEndGameState(EEndGameState NewEndGameState)
{
	if (!HasAuthority()
		|| NewEndGameState == EndGameStateInternal)
	{
		// No changes needed
		return;
	}

	EndGameStateInternal = NewEndGameState;
	ApplyEndGameState();
}

// Called on client when End-Game player status is changed
void AMyPlayerState::OnRep_EndGameState()
{
	ApplyEndGameState();
}

// Applies currently changed End-Game state for this player
void AMyPlayerState::ApplyEndGameState()
{
	AMyGameStateBase::Get().TrySetEndGameState();

	if (OnEndGameStateChanged.IsBound())
	{
		OnEndGameStateChanged.Broadcast(EndGameStateInternal);
	}
}

/*********************************************************************************************
 * Nickname
 ********************************************************************************************* */

// Called on server when settings are saved to apply new player name
void AMyPlayerState::ServerSetPlayerName_Implementation(FName NewName)
{
	SetPlayerName(NewName.ToString());
}

// Sets saved human name to config property
void AMyPlayerState::SetSavedPlayerName(FName NewName)
{
	if (SavedPlayerNameInternal != NewName)
	{
		SavedPlayerNameInternal = NewName;

		SetPlayerName(SavedPlayerNameInternal.ToString());
	}
}

// Applies default AI name based on current character ID like "AI 0", "AI 1" etc
void AMyPlayerState::SetDefaultBotName()
{
	if (!HasAuthority()
	    || !IsABot())
	{
		// Is not bot
		return;
	}

	const int32 CharacterID = GetPlayerCharacterChecked().GetPlayerId();
	const FString AIName = FString::Printf(TEXT("AI %s"), *FString::FromInt(CharacterID));
	if (GetPlayerName() != AIName)
	{
		SetPlayerName(AIName);
	}
}

// Overrides base method to additionally set player name on server and broadcast it
void AMyPlayerState::SetPlayerName(const FString& S)
{
	if (S == GetPlayerName())
	{
		return;
	}

	if (HasAuthority())
	{
		Super::SetPlayerName(S);
	}
	else
	{
		ServerSetPlayerName(*S);
		ApplyPlayerName(); // apply locally
	}
}

// Applies and broadcasts player nam
void AMyPlayerState::ApplyPlayerName()
{
	const FName PlayerNameCustom = *GetPlayerName();

	if (OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(PlayerNameCustom);
	}
}

// Called on client when custom player name is changed
void AMyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	ApplyPlayerName();
}

/*********************************************************************************************
 * Is Character Dead
 ********************************************************************************************* */

// Called when character dead status is changed: character was killed or revived
void AMyPlayerState::SetCharacterDead(bool bIsDead)
{
	if (!HasAuthority()
	    || bIsCharacterDeadInternal == bIsDead)
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
	if (HasAuthority())
	{
		AGeneratedMap::Get().OnPostDestroyedLevelActors.AddUniqueDynamic(this, &ThisClass::OnPostCharacterDead);
	}

	if (OnCharacterDeadChanged.IsBound())
	{
		OnCharacterDeadChanged.Broadcast(bIsCharacterDeadInternal);
	}
}

// Is called at the end of frame when this character received dead status
void AMyPlayerState::OnPostCharacterDead_Implementation(const FCells& Cells)
{
	if (bIsCharacterDeadInternal)
	{
		AGeneratedMap::Get().OnPostDestroyedLevelActors.RemoveAll(this);

		UpdateEndGameState();
	}
}

/*********************************************************************************************
 * Is Human / Bot
 ********************************************************************************************* */

// Applies bot status, overloads engine's APlayerState::SetIsABot(bool) that is not virtual and not exposed to blueprints
void AMyPlayerState::SetIsABot()
{
	if (!HasAuthority()
	    || IsABot())
	{
		return;
	}

	Super::SetIsABot(true);
	ApplyIsABot();
}

// Applies human status
void AMyPlayerState::SetIsHuman()
{
	if (!HasAuthority()
	    || !IsABot())
	{
		return;
	}

	Super::SetIsABot(false);
	ApplyIsABot();
}

// Called on client when APlayerState::bIsABot is changed
void AMyPlayerState::OnRep_IsABot()
{
	ApplyIsABot();
}

// Applies and broadcasts IsABot status
void AMyPlayerState::ApplyIsABot()
{
	if (OnIsABotChanged.IsBound())
	{
		OnIsABotChanged.Broadcast(IsABot());
	}
}

/*********************************************************************************************
 * Player ID (0, 1, 2, 3)
 ********************************************************************************************* */

// Applies ID from order of player controllers, is always 0, 1, 2, 3
void AMyPlayerState::SetHumanId(APlayerController* PlayerController)
{
	if (!HasAuthority()
	    && IsABot())
	{
		// Is not human
		return;
	}

	const AMyPlayerController* MyPC = Cast<AMyPlayerController>(PlayerController);
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const AMyGameModeBase* MyGameMode = UMyBlueprintFunctionLibrary::GetMyGameMode();
	const int32 NewPlayerId = MyGameMode ? MyGameMode->GetPlayerControllerIndex(MyPC) : INDEX_NONE;
	if (!ensureMsgf(NewPlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'NewPlayerId' can not be assigned!"), __LINE__, __FUNCTION__)
	    || NewPlayerId == GetPlayerId())
	{
		return;
	}

	SetPlayerId(NewPlayerId);
	ApplyPlayerId();
}

// Applies ID from order of spawned characters on level, is always 0, 1, 2, 3
void AMyPlayerState::SetDefaultBotId()
{
	if (!HasAuthority()
	    || !IsABot())
	{
		// Is not bot
		return;
	}

	const UMapComponent* PlayerMapComponent = UMapComponent::GetMapComponent(GetPawn());
	const int32 NewPlayerId = ULevelActorsUtilsLibrary::GetIndexByLevelActor(PlayerMapComponent);
	if (!ensureMsgf(NewPlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'NewPlayerId' can not be assigned!"), __LINE__, __FUNCTION__)
	    || NewPlayerId == GetPlayerId())
	{
		return;
	}

	SetPlayerId(NewPlayerId);
	ApplyPlayerId();
}

// Called on client when player ID is changed
void AMyPlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();

	ApplyPlayerId();
}

// Applies and broadcasts player ID
void AMyPlayerState::ApplyPlayerId()
{
	if (OnPlayerIdChanged.IsBound())
	{
		OnPlayerIdChanged.Broadcast(GetPlayerId());
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when player state is initialized with assigned character
void AMyPlayerState::OnPlayerStateInit_Implementation()
{
	if (IsABot())
	{
		// Apply bot ID here while Human ID is called from Game Session
		SetDefaultBotId();
	}

	UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnPlayerStateInit(*this);
}

// Listens game settings to apply them once saved
void AMyPlayerState::OnSaveSettings_Implementation()
{
	const FName PendingPlayerName = GetPendingPlayerName();
	SetSavedPlayerName(PendingPlayerName);
}

// Listen game states to notify server about ending game for controlled player
void AMyPlayerState::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECGS::Menu:         // fallthrough
		case ECGS::GameStarting: // Fallthrough
		case ECGS::InGame:
		{
			SetCharacterDead(false);
			SetEndGameState(EEndGameState::None);
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

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EndGameStateInternal);
	DOREPLIFETIME(ThisClass, bIsCharacterDeadInternal);

	// APlayerState::bIsABot private property has replication condition as 'Initial'
	// Reset to default condition, so the same player state can change its type without respawn
	static const FName bIsABot_PrivateProperty = TEXT("bIsABot");
	ResetReplicatedLifetimeProperty(StaticClass(), Super::StaticClass(), bIsABot_PrivateProperty, COND_None, OutLifetimeProps);
}

// Called when the game starts
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
	}
}

// Register a player with the online subsystem
void AMyPlayerState::RegisterPlayerWithSession(bool bWasFromInvite)
{
	Super::RegisterPlayerWithSession(bWasFromInvite);

	SetIsHuman();

	// Apply custom player name from config if any
	if (IsPlayerStateLocallyControlled())
	{
		// Listen game settings to apply them once saved
		UMyGameUserSettings::Get().OnSaveSettings.AddUniqueDynamic(this, &ThisClass::OnSaveSettings);

		// Apply custom player name from config
		if (SavedPlayerNameInternal.IsNone())
		{
			SavedPlayerNameInternal = *UKismetSystemLibrary::GetPlatformUserName();
		}
		SetPlayerName(SavedPlayerNameInternal.ToString());
		SetPendingPlayerName(SavedPlayerNameInternal);
	}
}

// Unregister a player with the online subsystem
void AMyPlayerState::UnregisterPlayerWithSession()
{
	Super::UnregisterPlayerWithSession();

	SetIsABot();
}

// Is overridden to handle own OnRep functions for engine properties
void AMyPlayerState::PostRepNotifies()
{
	Super::PostRepNotifies();

	// Engine's APlayerState::bIsABot property is 'Replicated', but not 'ReplicatedUsing'
	// So, detect replication manually
	static TMap<TWeakObjectPtr<ThisClass>, bool> IsBotCachedMap;
	bool& bIsBotCachedRef = IsBotCachedMap.FindOrAdd(this);
	if (bIsBotCachedRef != IsABot())
	{
		bIsBotCachedRef = IsABot();
		OnRep_IsABot();
	}
}