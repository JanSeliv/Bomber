// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "GeneratedMap.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/ModularGameFeatureSettings.h"
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

// Updates current game state
void AMyGameStateBase::ApplyGameState()
{
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
	StartingTimerSecRemainInternal = NewStartingTimerSecRemain;

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
	InGameTimerSecRemainInternal = NewInGameTimerSecRemain;

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
			constexpr bool bKeepRegistered = true;
			GameFeaturesSubsystem.UnloadGameFeaturePlugin(GameFeatureURL, EmptyCallback, bKeepRegistered);
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