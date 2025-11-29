// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyPlayerState.h"

// Bomber
#include "AbilitySystem/Attributes/BmrHealthAttributeSet.h"
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "AdvancedIdentityLibrary.h"
#include "AdvancedSteamFriendsLibrary.h"
#include "Components/MapComponent.h"
#include "Controllers/MyPlayerController.h"
#include "DataAssets/PlayerDataAsset.h"
#include "GameFramework/MyGameModeBase.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyGameUserSettings.h"
#include "GeneratedMap.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameplayAbilitiesModule.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPlayerState)

AMyPlayerState::AMyPlayerState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create ASC on player state, so even if different character is possessed (like from mod), it will still have the same attributes and abilities
	AbilitySystemComponentInternal = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponentInternal->SetIsReplicated(true);
	AbilitySystemComponentInternal->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PowerupsSetInternal = CreateDefaultSubobject<UBmrPowerupsAttributeSet>(TEXT("PowerupsAttributeSet"));
	HealthSetInternal = CreateDefaultSubobject<UBmrHealthAttributeSet>(TEXT("HealthAttributeSet"));

	// Reset default value to -1 to avoid conflicts with first player of 0 ID
	SetPlayerId(INDEX_NONE);
}

// Returns true if this Player State is controlled by a locally controlled player
bool AMyPlayerState::IsPlayerStateLocallyControlled() const
{
	const APlayerController* PC = GetPlayerController();
	return PC && PC->IsLocalPlayerController();
}

// Returns owner human or bot character
APlayerCharacter* AMyPlayerState::GetPlayerCharacter() const
{
	return GetPawn<APlayerCharacter>();
}

// Returns always valid owner (human or bot), or crash if nullptr
APlayerCharacter& AMyPlayerState::GetPlayerCharacterChecked() const
{
	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	checkf(PlayerCharacter, TEXT("ERROR: [%i] %hs:\n'PlayerCharacter' is null!"), __LINE__, __FUNCTION__);
	return *PlayerCharacter;
}

// Returns ability system component that is used to manage abilities and attributes for owned player, crash if nullptr
UAbilitySystemComponent& AMyPlayerState::GetAbilitySystemComponentChecked() const
{
	checkf(AbilitySystemComponentInternal, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponentInternal' is null!"), __LINE__, __FUNCTION__);
	return *AbilitySystemComponentInternal;
}

// Initializes all attributes with default values
void AMyPlayerState::ApplyDefaultAttributes()
{
	if (!HasAuthority())
	{
		return;
	}

	checkf(AbilitySystemComponentInternal, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponentInternal' is null!"), __LINE__, __FUNCTION__);

	// Initialize all attributes with default values
	const UAbilitySystemGlobals* AbilityGlobals = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals();
	const FAttributeSetInitter* AttributeSetInitter = AbilityGlobals ? AbilityGlobals->GetAttributeSetInitter() : nullptr;
	if (ensureMsgf(AttributeSetInitter, TEXT("ASSERT: [%i] %hs:\n'AttributeSetInitter' is null!"), __LINE__, __FUNCTION__))
	{
		static const FName GroupName = TEXT("Default");
		AttributeSetInitter->InitAttributeSetDefaults(AbilitySystemComponentInternal, GroupName, /*Level*/ 1, /*bInitialInit*/ true);
	}
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
	if (CurrentGameState == ECGS::None // is not valid game state, nullptr or not fully initialized
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
		const int32 PlayerNum = UMyBlueprintFunctionLibrary::GetAlivePlayersNum(EPlayerType::Any);

		if (IsCharacterDead())
		{
			if (PlayerNum <= 0)
			{
				// Draw: last players were blasted together
				return EEndGameState::Draw;
			}

			// Lose: player is dead, or Honor Loss if player has killed anyone else before dying
			return OpponentsKilledNumInternal > 0 ? EEndGameState::HonorLoss : EEndGameState::Lose;
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, EndGameStateInternal, this);

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
	// Try to end the game globally for all players
	if (EndGameStateInternal != EEndGameState::None)
	{
		if (UMyBlueprintFunctionLibrary::GetAlivePlayersNum(EPlayerType::Any) <= 1 // no characters to play with
		    || UMyBlueprintFunctionLibrary::GetAlivePlayersNum(EPlayerType::Human) == 0) // all human players are dead
		{
			AMyGameStateBase::Get().SetGameState(ECGS::EndGame);
		}
	}

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

// Is created on expose code-only GetOldPlayerName() base method to blueprints to get locally the player name on each nickname change
FName AMyPlayerState::GetPendingPlayerName() const
{
	const FName OldPlayerName = *GetOldPlayerName();
	return !OldPlayerName.IsNone() ? OldPlayerName : SavedPlayerNameInternal;
}

// Sets saved human name to config property
void AMyPlayerState::SetSavedPlayerName(FName NewName)
{
	if (SavedPlayerNameInternal == NewName
	    || !IsPlayerStateLocallyControlled())
	{
		return;
	}

	SavedPlayerNameInternal = NewName;

	SetPlayerName(SavedPlayerNameInternal.ToString());
}

// Attempts to assign default nickname
void AMyPlayerState::SetDefaultPlayerName()
{
	if (!HasAuthority())
	{
		return;
	}

	FString NewName = TEXT("");
	const EPlayerType PlayerType = GetPlayerType();
	switch (PlayerType)
	{
		case EPlayerType::Bot:
		{
			const int32 CharacterID = GetPlayerId();
			const FString AIName = FString::Printf(TEXT("AI %s"), *FString::FromInt(CharacterID));
			if (GetPlayerName() != AIName)
			{
				NewName = AIName;
			}
			break;
		}

		case EPlayerType::Human:
		{
			if (IsPlayerStateLocallyControlled())
			{
				// First, try to obtain player name from the OS
				NewName = UKismetSystemLibrary::GetPlatformUserName();
			}

			// Then, try to obtain player name from online subsystem
			if (UAdvancedSteamFriendsLibrary::IsOverlayEnabled())
			{
				FString OnlinePlayerName = TEXT("");
				UAdvancedIdentityLibrary::GetPlayerNickname(this, GetUniqueId(), /*out*/ OnlinePlayerName);
				if (!OnlinePlayerName.IsEmpty())
				{
					NewName = OnlinePlayerName;
				}
			}
			break;
		}

		default:
			break;
	}

	// New default name is set, so save it to config (if local)
	if (IsPlayerStateLocallyControlled())
	{
		SetSavedPlayerName(*NewName);
	}
	else
	{
		// Set name directly (without config)
		SetPlayerName(NewName);
	}
}

// Overrides base method to additionally set player name on server and broadcast it
void AMyPlayerState::SetPlayerName(const FString& NewPlayerName)
{
	if (NewPlayerName == GetPlayerName()
	    || NewPlayerName.IsEmpty())
	{
		return;
	}

	// First, apply new nickname locally
	Super::SetPlayerName(NewPlayerName);
	ApplyPlayerName();

	if (!HasAuthority()
	    && IsPlayerStateLocallyControlled())
	{
		// Let server know about new nickname, so it replicates to all clients
		ServerSetPlayerName(*NewPlayerName);
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsCharacterDeadInternal, this);

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
		// @TODO JanSeliv 5oWCcakc - Implement the player state manager to avoid using timer here
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnPostCharacterDead();
		}));
	}

	if (OnCharacterDeadChanged.IsBound())
	{
		OnCharacterDeadChanged.Broadcast(bIsCharacterDeadInternal);
	}
}

// Is called at the end of frame when this character received dead status
void AMyPlayerState::OnPostCharacterDead_Implementation()
{
	if (bIsCharacterDeadInternal)
	{
		UpdateEndGameState();
	}
}

/*********************************************************************************************
 * Killed Opponents Num
 *********************************************************************************************/

// Called when an opponent is killed
void AMyPlayerState::SetOpponentKilled(const class APlayerCharacter* KilledOpponent)
{
	if (!HasAuthority()
	    || !KilledOpponent
	    || KilledOpponent == GetPawn()) // is killed by itself
	{
		return;
	}

	const int32 NewValue = OpponentsKilledNumInternal + 1;
	SetOpponentKilledNum(NewValue);
}

void AMyPlayerState::SetOpponentKilledNum(int32 NewOpponentsKilledNum)
{
	if (!HasAuthority()
	    || NewOpponentsKilledNum == OpponentsKilledNumInternal)
	{
		return;
	}

	OpponentsKilledNumInternal = NewOpponentsKilledNum;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OpponentsKilledNumInternal, this);

	ApplyOpponentsKilledNum();
}

// Called on client when Opponents Killed Num changes
void AMyPlayerState::OnRep_OpponentsKilledNum()
{
	ApplyOpponentsKilledNum();
}

// Applies and broadcasts Opponents Killed Num changes
void AMyPlayerState::ApplyOpponentsKilledNum()
{
	if (OnOpponentsKilledNumChanged.IsBound())
	{
		OnOpponentsKilledNumChanged.Broadcast(OpponentsKilledNumInternal);
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
	// Depending on player type, set different replication mode for ASC: bots dont need to replicate all effects, so use Minimal mode
	const EGameplayEffectReplicationMode ReplicationMode = IsABot() ? EGameplayEffectReplicationMode::Minimal : EGameplayEffectReplicationMode::Mixed;
	GetAbilitySystemComponentChecked().SetReplicationMode(ReplicationMode);

	if (OnPlayerTypeChanged.IsBound())
	{
		OnPlayerTypeChanged.Broadcast(GetPlayerType());
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

// Is called on server and clients when new owned pawn is possessed or changed
void AMyPlayerState::OnPawnChanged_Implementation(APawn* NewPawn)
{
	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, NewPawn);
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

	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, GetPawn());

	ApplyIsABot();

	UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.Broadcast_OnPlayerStateInit(*this);

	if (IsPlayerStateLocallyControlled())
	{
		// Listen game settings to apply them once saved
		UMyGameUserSettings::Get().OnSaveSettings.AddUniqueDynamic(this, &ThisClass::OnSaveSettings);

		// Apply custom player name from config
		SetPlayerName(SavedPlayerNameInternal.ToString());
		if (SavedPlayerNameInternal.IsNone())
		{
			// Game is firstly launched, update config with default name
			SetDefaultPlayerName();
			SaveConfig();
		}
	}
}

// Listen game states to notify server about ending game for controlled player
void AMyPlayerState::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (CurrentGameState)
	{
		case ECGS::Menu: // Fallthrough
		case ECGS::GameStarting: // Fallthrough
		case ECGS::InGame:
		{
			SetCharacterDead(false);
			SetOpponentKilledNum(0);
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

// Listens game settings to apply them once saved
void AMyPlayerState::OnSaveSettings_Implementation()
{
	const FName PendingPlayerName = GetPendingPlayerName();
	SetSavedPlayerName(PendingPlayerName);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AbilitySystemComponentInternal, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EndGameStateInternal, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsCharacterDeadInternal, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OpponentsKilledNumInternal, Params);

	// Override APlayerState's COND_InitialOnly properties with default params to allow updates on reused instances without requiring respawn
	DOREPLIFETIME_OVERRIDE(Super, PlayerId, Params);
	DOREPLIFETIME_OVERRIDE(Super, bIsABot, Params);
	DOREPLIFETIME_OVERRIDE(Super, bIsInactive, Params);
	DOREPLIFETIME_OVERRIDE(Super, UniqueId, Params);
}

// This is called only in the gameplay before calling begin play
void AMyPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		ApplyDefaultAttributes();

		const UPlayerDataAsset& PlayerDataAsset = UPlayerDataAsset::Get();
		const int32 StartupAbilitiesNum = PlayerDataAsset.GetStartupAbilitiesNum();
		for (int32 Idx = 0; Idx < StartupAbilitiesNum; ++Idx)
		{
			const FGameplayAbilitySpec AbilitySpec = PlayerDataAsset.GetStartupAbility(Idx);
			GetAbilitySystemComponentChecked().GiveAbility(AbilitySpec);
		}
	}
}

// Called when the game starts
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Is overridden to prevent the player state from being destroyed to be able to reuse it by bots
void AMyPlayerState::OnDeactivated()
{
	// Do not call super to avoid destroying the player state
	return;
}

// Register a player with the online subsystem
void AMyPlayerState::RegisterPlayerWithSession(bool bWasFromInvite)
{
	if (!GetUniqueId().IsValid())
	{
		// Network id is not valid: player likely has left the session
		return;
	}

	Super::RegisterPlayerWithSession(bWasFromInvite);

	SetIsHuman();
}

// Unregister a player with the online subsystem
void AMyPlayerState::UnregisterPlayerWithSession()
{
	Super::UnregisterPlayerWithSession();

	const UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	// Human player left session, so set it as bot
	SetIsABot();

	// Reset player name to default
	SetDefaultPlayerName();

	// Reset network id, bots should have it empty
	if (GetUniqueId().IsValid())
	{
		SetUniqueId(FUniqueNetIdRepl());
	}
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