// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"

// Bomber
#include "Components/GameDifficultyManagerComponent.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/ModularGameFeatureSettings.h"
#include "GeneratedMap.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MyGameStateBase)

// Default constructor
AMyGameStateBase::AMyGameStateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GameDifficultyManagerInternal = CreateDefaultSubobject<UGameDifficultyManagerComponent>(TEXT("GameFeaturesManager"));
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

	if (NewGameState == ECurrentGameState::GameStarting
	    && !CanStartGame())
	{
		// Game is not allowed to start at current moment
		return false;
	}

	// Don't allow to change the game state if local character is not initialized or destroyed
	const APlayerCharacter* LocalChar = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter();
	return UGlobalEventsSubsystem::Get().OnCharactersReadyHandler.IsCharacterReady(LocalChar);
}

// Returns the AMyGameState::CurrentGameState property.
void AMyGameStateBase::SetGameState(ECurrentGameState NewGameState)
{
	if (!HasAuthority()
	    || !CanChangeGameState(NewGameState))
	{
		return;
	}

	ReplicatedGameStateInternal = NewGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedGameStateInternal, this);

	ApplyGameState();
}

// Returns the Game State that is currently applied
ECurrentGameState AMyGameStateBase::GetCurrentGameState()
{
	if (const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		return MyGameState->LocalGameStateInternal;
	}
	return ECurrentGameState::None;
}

// Returns the Game State that was applied before the current one
ECurrentGameState AMyGameStateBase::GetPreviousGameState()
{
	if (const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		return MyGameState->LocalPreviousGameStateInternal;
	}
	return ECurrentGameState::None;
}

// Returns true if the match can be started or restarted
bool AMyGameStateBase::CanStartGame() const
{
	if (LocalGameStateInternal == ECGS::GameStarting)
	{
		// The game is already starting (3-2-1)
		return false;
	}

	if (UMultiplayerUtilsLibrary::IsMultiplayerGame())
	{
		// In multiplayer, the game can be started only when it is ended or not running yet
		return LocalGameStateInternal == ECurrentGameState::EndGame
		       || LocalGameStateInternal == ECurrentGameState::Menu;
	}

	// In singleplayer, the game can be started or restarted at any time
	return true;
}

// Updates current game state
void AMyGameStateBase::ApplyGameState()
{
	TRACE_BOOKMARK(TEXT("%s"), *UEnum::GetValueAsString(ReplicatedGameStateInternal));

	LocalPreviousGameStateInternal = LocalGameStateInternal;
	LocalGameStateInternal = ReplicatedGameStateInternal;

	StopInGameCountdown();
	StopStartingCountdown();

	switch (LocalGameStateInternal)
	{
		case ECGS::GameStarting:
			TriggerStartingCountdown();
			break;
		case ECGS::InGame:
			TriggerInGameCountdown();
			break;
		default:
			break;
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
 * Starting Timer
 * 3-2-1-GO
 ********************************************************************************************* */

// Sets the left second of the 'Three-two-one-GO' timer
void AMyGameStateBase::SetStartingTimerSecondsRemain(float NewStartingTimerSecRemain)
{
	StartingTimerSecRemainInternal = FMath::Max(NewStartingTimerSecRemain, 0.f);

	if (OnStartingTimerSecRemainChanged.IsBound())
	{
		OnStartingTimerSecRemainChanged.Broadcast(StartingTimerSecRemainInternal);
	}
}

// Starts counting the 3-2-1-GO timer when match is starting, can be called both on the server and clients
void AMyGameStateBase::TriggerStartingCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetStartingTimerSecondsRemain(UGameStateDataAsset::Get().GetStartingCountdown());

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(StartingTimerInternal, this, &ThisClass::OnStartingTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the Starting timer and stops counting it
void AMyGameStateBase::StopStartingCountdown()
{
	if (StartingTimerInternal.IsValid())
	{
		GetWorldTimerManager().ClearTimer(StartingTimerInternal);
	}
}

// Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer, both on the server and clients
void AMyGameStateBase::OnStartingTimerTick()
{
	const float NewValue = StartingTimerSecRemainInternal - DefaultTimerIntervalSec;
	SetStartingTimerSecondsRemain(NewValue);

	if (IsStartingTimerElapsed())
	{
		StopStartingCountdown();
		SetGameState(ECurrentGameState::InGame);
	}
}

/*********************************************************************************************
 * In-Game Timer
 * Runs during the match (120...0)
 ********************************************************************************************* */

// Sets the left second to the end of the match
void AMyGameStateBase::SetInGameTimerSecondsRemain(float NewInGameTimerSecRemain)
{
	InGameTimerSecRemainInternal = FMath::Max(NewInGameTimerSecRemain, 0.f);

	if (OnInGameTimerSecRemainChanged.IsBound())
	{
		OnInGameTimerSecRemainChanged.Broadcast(InGameTimerSecRemainInternal);
	}
}

// Starts counting the (120...0) timer during the match, can be called both on the server and clients
void AMyGameStateBase::TriggerInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetInGameTimerSecondsRemain(UGameStateDataAsset::Get().GetInGameCountdown());

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(InGameTimerInternal, this, &ThisClass::OnInGameTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the In-Game timer and stops counting it
void AMyGameStateBase::StopInGameCountdown()
{
	if (InGameTimerInternal.IsValid())
	{
		GetWorldTimerManager().ClearTimer(InGameTimerInternal);
	}
}

// Is called once a second during the In-Game state to decrement the match timer, both on the server and clients
void AMyGameStateBase::OnInGameTimerTick()
{
	const float NewValue = InGameTimerSecRemainInternal - DefaultTimerIntervalSec;
	SetInGameTimerSecondsRemain(NewValue);

	// @todo JanSeliv baYkHels Adjust hardcoded value to match the duration of the EndGame SFX from meta sound
	{
		constexpr float SoundDuration = 10.f;
		const UWorld* World = GetWorld();
		checkf(World, TEXT("ERROR: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__);
		const float Tolerance = DefaultTimerIntervalSec - World->GetDeltaSeconds();
		if (FMath::IsNearlyEqual(InGameTimerSecRemainInternal, SoundDuration, Tolerance))
		{
			USoundsSubsystem::Get().PlayEndGameCountdownSFX();
		}
	}

	if (IsInGameTimerElapsed())
	{
		StopInGameCountdown();
		SetGameState(ECurrentGameState::EndGame);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedGameStateInternal, Params);
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

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(true, UModularGameFeatureSettings::Get().GetModularGameFeatures());

	BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady);
}

// Overridable function called whenever this actor is being removed from a level
void AMyGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(false, UModularGameFeatureSettings::Get().GetModularGameFeatures());
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