// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "GeneratedMap.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/ModularGameFeatureSettings.h"
#include "GameFramework/MyPlayerState.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "GameFeaturesSubsystem.h"
#include "TimerManager.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameStateBase)

// Default constructor
AMyGameStateBase::AMyGameStateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Returns the current game state, it will crash if can't be obtained, should be used only when the game is running
AMyGameStateBase& AMyGameStateBase::Get()
{
	AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	checkf(MyGameState, TEXT("ERROR: [%i] %s:\n'MyGameState' is null!"), __LINE__, *FString(__FUNCTION__));
	return *MyGameState;
}

/*********************************************************************************************
 * Current Game State enum
 ********************************************************************************************* */

// Returns true if current game state can be eventually changed
bool AMyGameStateBase::CanChangeGameState(ECurrentGameState NewGameState) const
{
	if (LocalGameStateInternal == NewGameState)
	{
		return false;
	}

	// Don't allow to change the game state if local character is not initialized or destroyed
	const APlayerCharacter* LocalChar = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	return UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.IsCharacterReady(LocalChar);
}

// Returns the AMyGameState::CurrentGameState property.
void AMyGameStateBase::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	if (!CanChangeGameState(NewGameState))
	{
		return;
	}

	// Update replicated state now, not waiting for the next tick
	// So, the client will receive replicated state in first order in comparing with other replicates
	ForceNetUpdate();

	ReplicatedGameStateInternal = NewGameState;
	ApplyGameState();
}

// Returns the AMyGameStateBase::CurrentGameState property
ECurrentGameState AMyGameStateBase::GetCurrentGameState()
{
	if (const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		return MyGameState->LocalGameStateInternal;
	}
	return ECurrentGameState::None;
}

// Updates current game state
void AMyGameStateBase::ApplyGameState()
{
	LocalGameStateInternal = ReplicatedGameStateInternal;

	if (LocalGameStateInternal == ECGS::GameStarting)
	{
		TriggerCountdowns();
	}

	// Notify listeners
	const UGlobalEventsSubsystem::FOnGameStateChanged& OnGameStateChanged = UGlobalEventsSubsystem::Get().BP_OnGameStateChanged;
	if (OnGameStateChanged.IsBound())
	{
		OnGameStateChanged.Broadcast(LocalGameStateInternal);
	}
}

// Called on the AMyGameState::CurrentGameState property updating.
void AMyGameStateBase::OnRep_CurrentGameState()
{
	if (CanChangeGameState(ReplicatedGameStateInternal))
	{
		ApplyGameState();
	}
}

/*********************************************************************************************
 * End-Game State enum
 * Result of finished match (Win, Lose or Draw)
 ********************************************************************************************* */

// Try to register the End-Game state
void AMyGameStateBase::TrySetEndGameState()
{
	bWantsUpdateEndStateInternal = true;
}

// Is called during the In-Game state to try to register the End-Game state
void AMyGameStateBase::UpdateEndGameStates()
{
	if (!bWantsUpdateEndStateInternal)
	{
		return;
	}

	bWantsUpdateEndStateInternal = false;

	if (UMyBlueprintFunctionLibrary::GetAlivePlayersNum() <= 1)
	{
		ServerSetGameState(ECGS::EndGame);
	}
}

/*********************************************************************************************
 * Starting Timer
 * 3-2-1-GO
 ********************************************************************************************* */

// Sets the left second of the 'Three-two-one-GO' timer
void AMyGameStateBase::SetStartingTimerSecondsRemain(float NewStartingTimerSecRemain)
{
	StartingTimerSecRemainInternal = NewStartingTimerSecRemain;
	ApplyStartingTimerSecondsRemain();
}

// Is called on client when the 'Three-two-one-GO' timer was updated
void AMyGameStateBase::OnRep_StartingTimerSecRemain()
{
	ApplyStartingTimerSecondsRemain();
}

// Updates current starting timer seconds remain
void AMyGameStateBase::ApplyStartingTimerSecondsRemain()
{
	if (OnStartingTimerSecRemainChanged.IsBound())
	{
		OnStartingTimerSecRemainChanged.Broadcast(StartingTimerSecRemainInternal);
	}
}

// Is called during the Game Starting state to handle the 'Three-two-one-GO' timer
void AMyGameStateBase::DecrementStartingCountdown()
{
	if (LocalGameStateInternal != ECGS::GameStarting)
	{
		return;
	}

	const float NewValue = StartingTimerSecRemainInternal - UGameStateDataAsset::Get().GetTickInterval();
	SetStartingTimerSecondsRemain(NewValue);

	if (IsStartingTimerElapsed())
	{
		ServerSetGameState(ECurrentGameState::InGame);
	}
}

/*********************************************************************************************
 * In-Game Timer
 * Runs during the match (120...0)
 ********************************************************************************************* */

// Sets the left second to the end of the match
void AMyGameStateBase::SetInGameTimerSecondsRemain(float NewInGameTimerSecRemain)
{
	InGameTimerSecRemainInternal = NewInGameTimerSecRemain;
	ApplyInGameTimerSecondsRemain();
}

// Is called on client when in-match timer was updated
void AMyGameStateBase::OnRep_InGameTimerSecRemain()
{
	ApplyInGameTimerSecondsRemain();
}

// Updates current in-match timer seconds remain
void AMyGameStateBase::ApplyInGameTimerSecondsRemain()
{
	if (OnInGameTimerSecRemainChanged.IsBound())
	{
		OnInGameTimerSecRemainChanged.Broadcast(InGameTimerSecRemainInternal);
	}
}

// Is called during the In-Game state to handle time consuming for the current match
void AMyGameStateBase::DecrementInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World || LocalGameStateInternal != ECGS::InGame)
	{
		return;
	}

	const float NewValue = InGameTimerSecRemainInternal - UGameStateDataAsset::Get().GetTickInterval();
	SetInGameTimerSecondsRemain(NewValue);

	if (IsInGameTimerElapsed())
	{
		ServerSetGameState(ECurrentGameState::EndGame);
	}
	else
	{
		// @todo JanSeliv baYkHels Adjust hardcoded value to match the duration of the EndGame SFX from meta sound
		const float Tolerance = UGameStateDataAsset::Get().GetTickInterval() - World->GetDeltaSeconds();
		constexpr float SoundDuration = 10.f;
		if (FMath::IsNearlyEqual(InGameTimerSecRemainInternal, SoundDuration, Tolerance))
		{
			USoundsSubsystem::Get().PlayEndGameCountdownSFX();
		}
	}
}

// Called to starting counting different time in the game
void AMyGameStateBase::TriggerCountdowns()
{
	const UWorld* World = GetWorld();
	if (!World
	    || !HasAuthority())
	{
		return;
	}

	SetStartingTimerSecondsRemain(UGameStateDataAsset::Get().GetStartingCountdown());
	SetInGameTimerSecondsRemain(UGameStateDataAsset::Get().GetInGameCountdown());

	constexpr bool bInLoop = true;
	const float InRate = UGameStateDataAsset::Get().GetTickInterval();
	World->GetTimerManager().SetTimer(CountdownTimerInternal, this, &ThisClass::OnCountdownTimerTicked, InRate, bInLoop);
}

// Is called each UGameStateDataAsset::TickInternal to count different time in the game
void AMyGameStateBase::OnCountdownTimerTicked()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	switch (LocalGameStateInternal)
	{
		case ECGS::GameStarting:
			DecrementStartingCountdown();
			break;
		case ECGS::InGame:
			DecrementInGameCountdown();
			UpdateEndGameStates();
			break;
		default:
			World->GetTimerManager().ClearTimer(CountdownTimerInternal);
			break;
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns amount of players (host + clients) playing this game
int32 AMyGameStateBase::GetPlayersInMultiplayerNum() const
{
	int32 PlayersNum = 0;
	for (const APlayerState* PlayerStateIt : PlayerArray)
	{
		if (PlayerStateIt && !PlayerStateIt->IsABot())
		{
			++PlayersNum;
		}
	}
	return PlayersNum;
}

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReplicatedGameStateInternal);
	DOREPLIFETIME(ThisClass, StartingTimerSecRemainInternal);
	DOREPLIFETIME(ThisClass, InGameTimerSecRemainInternal);
}

// This is called only in the gameplay before calling begin play
void AMyGameStateBase::PostInitializeComponents()
{
	// Register it to let modular feature to be dynamically added
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts
void AMyGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	SetGameFeaturesEnabled(true);

	BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady);
}

// Overridable function called whenever this actor is being removed from a level
void AMyGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	SetGameFeaturesEnabled(false);
}

// Enables or disable all game features
void AMyGameStateBase::SetGameFeaturesEnabled(bool bEnable)
{
	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	const TArray<FName>& GameFeaturesToEnable = UModularGameFeatureSettings::Get().GetModularGameFeatures();
	for (const FName GameFeatureName : GameFeaturesToEnable)
	{
		if (GameFeatureName.IsNone())
		{
			continue;
		}

		FString GameFeatureURL;
		GameFeaturesSubsystem.GetPluginURLByName(GameFeatureName.ToString(), /*out*/ GameFeatureURL);
		if (!ensureMsgf(!GameFeatureURL.IsEmpty(), TEXT("ASSERT: Can't load '%s' game feature"), *GameFeatureName.ToString()))
		{
			continue;
		}

		static const FGameFeaturePluginLoadComplete EmptyCallback{};
		if (bEnable)
		{
			GameFeaturesSubsystem.LoadAndActivateGameFeaturePlugin(GameFeatureURL, EmptyCallback);
		}
		else
		{
			GameFeaturesSubsystem.DeactivateGameFeaturePlugin(GameFeatureURL, EmptyCallback);
		}
	}
}

// Called when the local player character is spawned, possessed, and replicated
void AMyGameStateBase::OnLocalCharacterReady_Implementation(class APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	// Try update the game state when the local character is initialized, if not set yet
	if (CanChangeGameState(ReplicatedGameStateInternal))
	{
		ApplyGameState();
	}
}