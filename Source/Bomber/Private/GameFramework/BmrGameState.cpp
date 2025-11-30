// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrGameState.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Actors/BmrPawn.h"
#include "Components/BmrGameDifficultyManagerComponent.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataAssets/BmrModularGameFeatureSettings.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameState)

// Default constructor
ABmrGameState::ABmrGameState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GameDifficultyManager = CreateDefaultSubobject<UBmrGameDifficultyManagerComponent>(TEXT("GameFeaturesManager"));
}

// Returns the current game state, it will crash if can't be obtained, should be used only when the game is running
ABmrGameState& ABmrGameState::Get()
{
	ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState();
	checkf(MyGameState, TEXT("ERROR: [%i] %s:\n'MyGameState' is null!"), __LINE__, *FString(__FUNCTION__));
	return *MyGameState;
}

/*********************************************************************************************
 * Current Game State enum
 ********************************************************************************************* */

// Returns true if current game state can be eventually changed
bool ABmrGameState::CanChangeGameState(EBmrCurrentGameState NewGameState) const
{
	if (LocalGameState == NewGameState)
	{
		return false;
	}

	if (NewGameState == EBmrCurrentGameState::GameStarting
	    && !CanStartGame())
	{
		// Game is not allowed to start at current moment
		return false;
	}

	// Don't allow to change the game state if local character is not initialized or destroyed
	const ABmrPawn* LocalChar = UBmrBlueprintFunctionLibrary::GetLocalPawn();
	return UBmrGlobalEventsSubsystem::Get().ReadyHandler.IsReady(LocalChar);
}

// Returns the AMyGameState::CurrentGameState property.
void ABmrGameState::SetGameState(EBmrCurrentGameState NewGameState)
{
	if (!HasAuthority()
	    || !CanChangeGameState(NewGameState))
	{
		return;
	}

	ReplicatedGameState = NewGameState;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedGameState, this);

	ApplyGameState();
}

// Returns the Game State that is currently applied
EBmrCurrentGameState ABmrGameState::GetCurrentGameState()
{
	if (const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
	{
		return MyGameState->LocalGameState;
	}
	return EBmrCurrentGameState::None;
}

// Returns the Game State that was applied before the current one
EBmrCurrentGameState ABmrGameState::GetPreviousGameState()
{
	if (const ABmrGameState* MyGameState = UBmrBlueprintFunctionLibrary::GetGameState())
	{
		return MyGameState->LocalPreviousGameState;
	}
	return EBmrCurrentGameState::None;
}

// Returns true if the match can be started or restarted
bool ABmrGameState::CanStartGame() const
{
	if (LocalGameState == ECGS::GameStarting)
	{
		// The game is already starting (3-2-1)
		return false;
	}

	if (UMultiplayerUtilsLibrary::IsMultiplayerGame())
	{
		// In multiplayer, the game can be started only when it is ended or not running yet
		return LocalGameState == EBmrCurrentGameState::EndGame
		       || LocalGameState == EBmrCurrentGameState::Menu;
	}

	// In singleplayer, the game can be started or restarted at any time
	return true;
}

// Updates current game state
void ABmrGameState::ApplyGameState()
{
	TRACE_BOOKMARK(TEXT("%s"), *UEnum::GetValueAsString(ReplicatedGameState));

	LocalPreviousGameState = LocalGameState;
	LocalGameState = ReplicatedGameState;

	StopInGameCountdown();
	StopStartingCountdown();

	switch (LocalGameState)
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
	const UBmrGlobalEventsSubsystem::FOnGameStateChanged& OnGameStateChanged = UBmrGlobalEventsSubsystem::Get().BP_OnGameStateChanged;
	if (OnGameStateChanged.IsBound())
	{
		OnGameStateChanged.Broadcast(LocalGameState);
	}
}

// Called on the AMyGameState::CurrentGameState property updating.
void ABmrGameState::OnRep_CurrentGameState()
{
	if (CanChangeGameState(ReplicatedGameState))
	{
		ApplyGameState();
	}
}

/*********************************************************************************************
 * Starting Timer
 * 3-2-1-GO
 ********************************************************************************************* */

// Sets the left second of the 'Three-two-one-GO' timer
void ABmrGameState::SetStartingTimerSecondsRemain(float NewStartingTimerSecRemain)
{
	StartingTimerSecRemain = FMath::Max(NewStartingTimerSecRemain, 0.f);

	if (OnStartingTimerSecRemainChanged.IsBound())
	{
		OnStartingTimerSecRemainChanged.Broadcast(StartingTimerSecRemain);
	}
}

// Starts counting the 3-2-1-GO timer when match is starting, can be called both on the server and clients
void ABmrGameState::TriggerStartingCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetStartingTimerSecondsRemain(UBmrGameStateDataAsset::Get().GetStartingCountdown());

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(StartingTimer, this, &ThisClass::OnStartingTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the Starting timer and stops counting it
void ABmrGameState::StopStartingCountdown()
{
	if (StartingTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(StartingTimer);
	}
}

// Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer, both on the server and clients
void ABmrGameState::OnStartingTimerTick()
{
	const float NewValue = StartingTimerSecRemain - DefaultTimerIntervalSec;
	SetStartingTimerSecondsRemain(NewValue);

	if (IsStartingTimerElapsed())
	{
		StopStartingCountdown();
		SetGameState(EBmrCurrentGameState::InGame);
	}
}

/*********************************************************************************************
 * In-Game Timer
 * Runs during the match (120...0)
 ********************************************************************************************* */

// Sets the left second to the end of the match
void ABmrGameState::SetInGameTimerSecondsRemain(float NewInGameTimerSecRemain)
{
	InGameTimerSecRemain = FMath::Max(NewInGameTimerSecRemain, 0.f);

	if (OnInGameTimerSecRemainChanged.IsBound())
	{
		OnInGameTimerSecRemainChanged.Broadcast(InGameTimerSecRemain);
	}
}

// Starts counting the (120...0) timer during the match, can be called both on the server and clients
void ABmrGameState::TriggerInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetInGameTimerSecondsRemain(UBmrGameStateDataAsset::Get().GetInGameCountdown());

	constexpr bool bInLoop = true;
	World->GetTimerManager().SetTimer(InGameTimer, this, &ThisClass::OnInGameTimerTick, DefaultTimerIntervalSec, bInLoop);
}

// Clears the In-Game timer and stops counting it
void ABmrGameState::StopInGameCountdown()
{
	if (InGameTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(InGameTimer);
	}
}

// Is called once a second during the In-Game state to decrement the match timer, both on the server and clients
void ABmrGameState::OnInGameTimerTick()
{
	const float NewValue = InGameTimerSecRemain - DefaultTimerIntervalSec;
	SetInGameTimerSecondsRemain(NewValue);

	// @todo JanSeliv baYkHels Adjust hardcoded value to match the duration of the EndGame SFX from meta sound
	{
		constexpr float SoundDuration = 10.f;
		const UWorld* World = GetWorld();
		checkf(World, TEXT("ERROR: [%i] %hs:\n'World' is null!"), __LINE__, __FUNCTION__);
		const float Tolerance = DefaultTimerIntervalSec - World->GetDeltaSeconds();
		if (FMath::IsNearlyEqual(InGameTimerSecRemain, SoundDuration, Tolerance))
		{
			UBmrSoundsSubsystem::Get().PlayEndGameCountdownSFX();
		}
	}

	if (IsInGameTimerElapsed())
	{
		StopInGameCountdown();
		SetGameState(EBmrCurrentGameState::EndGame);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Returns properties that are replicated for the lifetime of the actor channel.
void ABmrGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedGameState, Params);
}

// This is called only in the gameplay before calling begin play
void ABmrGameState::PostInitializeComponents()
{
	// Register it to let modular feature to be dynamically added
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	Super::PostInitializeComponents();
}

// Called when the game starts
void ABmrGameState::BeginPlay()
{
	Super::BeginPlay();

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(true, UBmrModularGameFeatureSettings::Get().GetModularGameFeatures());

	BIND_ON_LOCAL_PAWN_READY(this, ThisClass::OnLocalPawnReady);
}

// Overridable function called whenever this actor is being removed from a level
void ABmrGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGameplayUtilsLibrary::SetGameFeaturesEnabled(false, UBmrModularGameFeatureSettings::Get().GetModularGameFeatures());
}

// Called when the local player character is spawned, possessed, and replicated
void ABmrGameState::OnLocalPawnReady_Implementation(class ABmrPawn* Pawn, int32 PlayerId)
{
	// Try update the game state when the local character is initialized, if not set yet
	if (CanChangeGameState(ReplicatedGameState))
	{
		ApplyGameState();
	}
}