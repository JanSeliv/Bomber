// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrPlayerState.h"

// Bomber
#include "AbilitySystem/Attributes/BmrHealthAttributeSet.h"
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "AdvancedIdentityLibrary.h"
#include "AdvancedSteamFriendsLibrary.h"
#include "Components/BmrMapComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrPlayerDataAsset.h"
#include "GameFramework/BmrGameMode.h"
#include "GameFramework/BmrGameState.h"
#include "GameFramework/BmrGameUserSettings.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameplayAbilitiesModule.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerState)

ABmrPlayerState::ABmrPlayerState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create ASC on player state, so even if different character is possessed (like from mod), it will still have the same attributes and abilities
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PowerupsSet = CreateDefaultSubobject<UBmrPowerupsAttributeSet>(TEXT("PowerupsAttributeSet"));
	HealthSet = CreateDefaultSubobject<UBmrHealthAttributeSet>(TEXT("HealthAttributeSet"));

	// Reset default value to -1 to avoid conflicts with first player of 0 ID
	SetPlayerId(INDEX_NONE);
}

// Returns true if this Player State is controlled by a locally controlled player
bool ABmrPlayerState::IsPlayerStateLocallyControlled() const
{
	const APlayerController* PC = GetPlayerController();
	return PC && PC->IsLocalPlayerController();
}

// Returns always valid owner (human or bot), or crash if nullptr
ABmrPawn& ABmrPlayerState::GetPawnChecked() const
{
	ABmrPawn* Pawn = GetPawn<ABmrPawn>();
	checkf(Pawn, TEXT("ERROR: [%i] %hs:\n'Pawn' is null!"), __LINE__, __FUNCTION__);
	return *Pawn;
}

// Returns ability system component that is used to manage abilities and attributes for owned player, crash if nullptr
UAbilitySystemComponent& ABmrPlayerState::GetAbilitySystemComponentChecked() const
{
	checkf(AbilitySystemComponent, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);
	return *AbilitySystemComponent;
}

// Initializes all attributes with default values
void ABmrPlayerState::ApplyDefaultAttributes()
{
	if (!HasAuthority())
	{
		return;
	}

	checkf(AbilitySystemComponent, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);

	// Initialize all attributes with default values
	const UAbilitySystemGlobals* AbilityGlobals = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals();
	const FAttributeSetInitter* AttributeSetInitter = AbilityGlobals ? AbilityGlobals->GetAttributeSetInitter() : nullptr;
	if (ensureMsgf(AttributeSetInitter, TEXT("ASSERT: [%i] %hs:\n'AttributeSetInitter' is null!"), __LINE__, __FUNCTION__))
	{
		static const FName GroupName = TEXT("Default");
		AttributeSetInitter->InitAttributeSetDefaults(AbilitySystemComponent, GroupName, /*Level*/ 1, /*bInitialInit*/ true);
	}
}

/*********************************************************************************************
 * End Game State
 ********************************************************************************************* */

// Tries to set new End-Game state for this player
void ABmrPlayerState::UpdateEndGameState()
{
	if (!HasAuthority())
	{
		return;
	}

	const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState();
	const EBmrCurrentGameState CurrentGameState = MyGameState ? MyGameState->GetCurrentGameState() : ECGS::None;
	if (CurrentGameState == ECGS::None // is not valid game state, nullptr or not fully initialized
	    || EndGameState != EBmrEndGameState::None) // end state was set already for current game
	{
		return;
	}

	// handle timer is elapsed
	if (CurrentGameState == ECGS::EndGame)
	{
		SetEndGameState(EBmrEndGameState::Draw);
		return;
	}

	// Game is running

	const EBmrEndGameState NewEndGameState = [&]
	{
		const int32 PlayerNum = UBmrBlueprintFunctionLibrary::GetAlivePlayersNum(EBmrPlayerType::Any);

		if (IsPlayerDead())
		{
			if (PlayerNum <= 0)
			{
				// Draw: last players were blasted together
				return EBmrEndGameState::Draw;
			}

			// Lose: player is dead, or Honor Loss if player has killed anyone else before dying
			return OpponentsKilledNum > 0 ? EBmrEndGameState::HonorLoss : EBmrEndGameState::Lose;
		}

		// Win: Is alive owner and is the last player
		return PlayerNum == 1 ? EBmrEndGameState::Win : EBmrEndGameState::None;
	}();

	SetEndGameState(NewEndGameState);
}

// Sets End-Game state to the specified one
void ABmrPlayerState::SetEndGameState(EBmrEndGameState NewEndGameState)
{
	if (!HasAuthority()
	    || NewEndGameState == EndGameState)
	{
		// No changes needed
		return;
	}

	EndGameState = NewEndGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, EndGameState, this);

	ApplyEndGameState();
}

// Called on client when End-Game player status is changed
void ABmrPlayerState::OnRep_EndGameState()
{
	ApplyEndGameState();
}

// Applies currently changed End-Game state for this player
void ABmrPlayerState::ApplyEndGameState()
{
	// Try to end the game globally for all players
	if (EndGameState != EBmrEndGameState::None)
	{
		// Notify that this player's end game state changed, BmrGameState will check conditions and decide whether to end the match
		FGameplayEventData EventData;
		EventData.EventTag = BmrGameplayTags::Event::Player_OnEndGame;
		EventData.Instigator = GetPawn();
		UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
	}

	if (OnEndGameStateChanged.IsBound())
	{
		OnEndGameStateChanged.Broadcast(EndGameState);
	}
}

/*********************************************************************************************
 * Nickname
 ********************************************************************************************* */

// Called on server when settings are saved to apply new player name
void ABmrPlayerState::ServerSetPlayerName_Implementation(FName NewName)
{
	SetPlayerName(NewName.ToString());
}

// Is created on expose code-only GetOldPlayerName() base method to blueprints to get locally the player name on each nickname change
FName ABmrPlayerState::GetPendingPlayerName() const
{
	const FName OldPlayerName = *GetOldPlayerName();
	return !OldPlayerName.IsNone() ? OldPlayerName : SavedPlayerName;
}

// Sets saved human name to config property
void ABmrPlayerState::SetSavedPlayerName(FName NewName)
{
	if (SavedPlayerName == NewName
	    || !IsPlayerStateLocallyControlled())
	{
		return;
	}

	SavedPlayerName = NewName;

	SetPlayerName(SavedPlayerName.ToString());
}

// Attempts to assign default nickname
void ABmrPlayerState::SetDefaultPlayerName()
{
	if (!HasAuthority())
	{
		return;
	}

	FString NewName = TEXT("");
	const EBmrPlayerType PlayerType = GetPlayerType();
	switch (PlayerType)
	{
		case EBmrPlayerType::Bot:
		{
			const int32 InPlayerId = GetPlayerId();
			const FString AIName = FString::Printf(TEXT("AI %s"), *FString::FromInt(InPlayerId));
			if (GetPlayerName() != AIName)
			{
				NewName = AIName;
			}
			break;
		}

		case EBmrPlayerType::Human:
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
void ABmrPlayerState::SetPlayerName(const FString& NewPlayerName)
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
void ABmrPlayerState::ApplyPlayerName()
{
	const FName PlayerNameCustom = *GetPlayerName();

	if (OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(PlayerNameCustom);
	}
}

// Called on client when custom player name is changed
void ABmrPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	ApplyPlayerName();
}

/*********************************************************************************************
 * Is Character Dead
 ********************************************************************************************* */

// Called when character dead status is changed: character was killed or revived
void ABmrPlayerState::SetPlayerDead(bool bIsDead)
{
	if (!HasAuthority()
	    || bIsPlayerDead == bIsDead)
	{
		return;
	}

	bIsPlayerDead = bIsDead;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsPlayerDead, this);

	ApplyIsPlayerDead();
}

// Called on client when character Dead status is changed
void ABmrPlayerState::OnRep_IsPlayerDead()
{
	ApplyIsPlayerDead();
}

// Applies and broadcasts Is Character Dead status
void ABmrPlayerState::ApplyIsPlayerDead()
{
	if (HasAuthority())
	{
		// @TODO JanSeliv 5oWCcakc - Implement the player state manager to avoid using timer here
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnPostPlayerDead();
		}));
	}

	if (OnPlayerDeadChanged.IsBound())
	{
		OnPlayerDeadChanged.Broadcast(bIsPlayerDead);
	}
}

// Is called at the end of frame when this character received dead status
void ABmrPlayerState::OnPostPlayerDead_Implementation()
{
	if (bIsPlayerDead)
	{
		UpdateEndGameState();
	}
}

/*********************************************************************************************
 * Killed Opponents Num
 *********************************************************************************************/

// Called when an opponent is killed
void ABmrPlayerState::SetOpponentKilled(const class ABmrPawn* KilledOpponent)
{
	if (!HasAuthority()
	    || !KilledOpponent
	    || KilledOpponent == GetPawn()) // is killed by itself
	{
		return;
	}

	const int32 NewValue = OpponentsKilledNum + 1;
	SetOpponentKilledNum(NewValue);
}

void ABmrPlayerState::SetOpponentKilledNum(int32 NewOpponentsKilledNum)
{
	if (!HasAuthority()
	    || NewOpponentsKilledNum == OpponentsKilledNum)
	{
		return;
	}

	OpponentsKilledNum = NewOpponentsKilledNum;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OpponentsKilledNum, this);

	ApplyOpponentsKilledNum();
}

// Called on client when Opponents Killed Num changes
void ABmrPlayerState::OnRep_OpponentsKilledNum()
{
	ApplyOpponentsKilledNum();
}

// Applies and broadcasts Opponents Killed Num changes
void ABmrPlayerState::ApplyOpponentsKilledNum()
{
	if (OnOpponentsKilledNumChanged.IsBound())
	{
		OnOpponentsKilledNumChanged.Broadcast(OpponentsKilledNum);
	}
}

/*********************************************************************************************
 * Is Human / Bot
 ********************************************************************************************* */

// Applies bot status, overloads engine's APlayerState::SetIsABot(bool) that is not virtual and not exposed to blueprints
void ABmrPlayerState::SetIsABot()
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
void ABmrPlayerState::SetIsHuman()
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
void ABmrPlayerState::OnRep_IsABot()
{
	ApplyIsABot();
}

// Applies and broadcasts IsABot status
void ABmrPlayerState::ApplyIsABot()
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
void ABmrPlayerState::SetHumanId(APlayerController* PlayerController)
{
	if (!HasAuthority()
	    && IsABot())
	{
		// Is not human
		return;
	}

	const ABmrPlayerController* MyPC = Cast<ABmrPlayerController>(PlayerController);
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %hs:\n'MyPC' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const ABmrGameMode* MyGameMode = UBmrBlueprintFunctionLibrary::GetGameMode();
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
void ABmrPlayerState::SetDefaultBotId()
{
	if (!HasAuthority()
	    || !IsABot())
	{
		// Is not bot
		return;
	}

	const UBmrMapComponent* PlayerMapComponent = UBmrMapComponent::GetMapComponent(GetPawn());
	const int32 NewPlayerId = UBmrActorUtilsLibrary::GetIndexByLevelActor(PlayerMapComponent);
	if (!ensureMsgf(NewPlayerId >= 0, TEXT("ASSERT: [%i] %hs:\n'NewPlayerId' can not be assigned!"), __LINE__, __FUNCTION__)
	    || NewPlayerId == GetPlayerId())
	{
		return;
	}

	SetPlayerId(NewPlayerId);
	ApplyPlayerId();
}

// Called on client when player ID is changed
void ABmrPlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();

	ApplyPlayerId();
}

// Applies and broadcasts player ID
void ABmrPlayerState::ApplyPlayerId()
{
	if (OnPlayerIdChanged.IsBound())
	{
		OnPlayerIdChanged.Broadcast(GetPlayerId());
	}
}

// Is called on server and clients when new owned pawn is possessed or changed
void ABmrPlayerState::OnPawnChanged_Implementation(APawn* NewPawn)
{
	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, NewPawn);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when player state is initialized with assigned character
void ABmrPlayerState::OnPlayerStateInit_Implementation()
{
	if (IsABot())
	{
		// Apply bot ID here while Human ID is called from Game Session
		SetDefaultBotId();
	}

	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, GetPawn());

	ApplyIsABot();

	UBmrGameplayMessageSubsystem::Get().ReadyHandler.Broadcast_OnPlayerStateInit(*this);

	if (IsPlayerStateLocallyControlled())
	{
		// Listen game settings to apply them once saved
		UBmrGameUserSettings::Get().OnSaveSettings.AddUniqueDynamic(this, &ThisClass::OnSaveSettings);

		// Apply custom player name from config
		SetPlayerName(SavedPlayerName.ToString());
		if (SavedPlayerName.IsNone())
		{
			// Game is firstly launched, update config with default name
			SetDefaultPlayerName();
			SaveConfig();
		}
	}
}

// Listen game states to notify server about ending game for controlled player
void ABmrPlayerState::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const EBmrCurrentGameState CurrentGameState = ABmrGameState::GetCurrentGameState();
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
			SetPlayerDead(false);
			SetOpponentKilledNum(0);
			SetEndGameState(EBmrEndGameState::None);
			break;
		}
		case EBmrCurrentGameState::EndGame:
		{
			UpdateEndGameState();
			break;
		}
		default:
			break;
	}
}

// Listens game settings to apply them once saved
void ABmrPlayerState::OnSaveSettings_Implementation()
{
	const FName PendingPlayerName = GetPendingPlayerName();
	SetSavedPlayerName(PendingPlayerName);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void ABmrPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AbilitySystemComponent, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EndGameState, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsPlayerDead, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OpponentsKilledNum, Params);

	// Override APlayerState's COND_InitialOnly properties with default params to allow updates on reused instances without requiring respawn
	DOREPLIFETIME_OVERRIDE(Super, PlayerId, Params);
	DOREPLIFETIME_OVERRIDE(Super, bIsABot, Params);
	DOREPLIFETIME_OVERRIDE(Super, bIsInactive, Params);
	DOREPLIFETIME_OVERRIDE(Super, UniqueId, Params);
}

// This is called only in the gameplay before calling begin play
void ABmrPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		ApplyDefaultAttributes();

		const UBmrPlayerDataAsset& PlayerDataAsset = UBmrPlayerDataAsset::Get();
		const int32 StartupAbilitiesNum = PlayerDataAsset.GetStartupAbilitiesNum();
		for (int32 Idx = 0; Idx < StartupAbilitiesNum; ++Idx)
		{
			const FGameplayAbilitySpec AbilitySpec = PlayerDataAsset.GetStartupAbility(Idx);
			GetAbilitySystemComponentChecked().GiveAbility(AbilitySpec);
		}
	}
}

// Called when the game starts
void ABmrPlayerState::BeginPlay()
{
	Super::BeginPlay();

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Is overridden to prevent the player state from being destroyed to be able to reuse it by bots
void ABmrPlayerState::OnDeactivated()
{
	// Do not call super to avoid destroying the player state
	return;
}

// Register a player with the online subsystem
void ABmrPlayerState::RegisterPlayerWithSession(bool bWasFromInvite)
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
void ABmrPlayerState::UnregisterPlayerWithSession()
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
void ABmrPlayerState::PostRepNotifies()
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