// Copyright (c) Yevhenii Selivanov

#include "GameFramework/MyGameStateBase.h"
//---
#include "GeneratedMap.h"
#include "Subsystems/SoundsSubsystem.h"
#include "GameFramework/MyPlayerState.h"
#include "DataAssets/GameStateDataAsset.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "GameFeaturesSubsystem.h"
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

// Returns the AMyGameStateBase::CurrentGameState property
ECurrentGameState AMyGameStateBase::GetCurrentGameState()
{
	if (const AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		return MyGameState->CurrentGameStateInternal;
	}
	return ECurrentGameState::None;
}

// Returns the AMyGameState::CurrentGameState property.
void AMyGameStateBase::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	if (NewGameState == CurrentGameStateInternal)
	{
		return;
	}

	CurrentGameStateInternal = NewGameState;
	ApplyGameState();
}

/* ---------------------------------------------------
 *		Protected
 * --------------------------------------------------- */

// Returns properties that are replicated for the lifetime of the actor channel.
void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameStateInternal);
	DOREPLIFETIME(ThisClass, StartingTimerSecRemainInternal);
	DOREPLIFETIME(ThisClass, InGameTimerSecRemainInternal);
}

// Called when the game starts
void AMyGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AGeneratedMap::Get().OnAnyCharacterDestroyed.AddDynamic(this, &ThisClass::OnAnyCharacterDestroyed);
	}

	SetGameFeaturesEnabled(true);
}

// Overridable function called whenever this actor is being removed from a level
void AMyGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	SetGameFeaturesEnabled(false);
}

// Updates current game state
void AMyGameStateBase::ApplyGameState()
{
	if (CurrentGameStateInternal == ECGS::GameStarting)
	{
		TriggerCountdowns();
	}

	// Notify listeners
	if (OnGameStateChanged.IsBound())
	{
		OnGameStateChanged.Broadcast(CurrentGameStateInternal);
	}
}

// Called on the AMyGameState::CurrentGameState property updating.
void AMyGameStateBase::OnRep_CurrentGameState()
{
	ApplyGameState();
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

	StartingTimerSecRemainInternal = UGameStateDataAsset::Get().GetStartingCountdown();
	InGameTimerSecRemainInternal = UGameStateDataAsset::Get().GetInGameCountdown();

	constexpr bool bInLoop = true;
	const float InRate = UGameStateDataAsset::Get().GetTickInterval();
	World->GetTimerManager().SetTimer(CountdownTimerInternal, this, &ThisClass::OnCountdownTimerTicked, InRate, bInLoop);

	USoundsSubsystem::Get().PlayStartGameCountdownSFX();
}

// Is called each UGameStateDataAsset::TickInternal to count different time in the game
void AMyGameStateBase::OnCountdownTimerTicked()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	switch (CurrentGameStateInternal)
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

// Is called during the Game Starting state to handle the 'Three-two-one-GO' timer
void AMyGameStateBase::DecrementStartingCountdown()
{
	if (CurrentGameStateInternal != ECGS::GameStarting)
	{
		return;
	}

	StartingTimerSecRemainInternal -= UGameStateDataAsset::Get().GetTickInterval();

	if (IsStartingTimerElapsed())
	{
		ServerSetGameState(ECurrentGameState::InGame);
	}
}

// Is called during the In-Game state to handle time consuming for the current match
void AMyGameStateBase::DecrementInGameCountdown()
{
	const UWorld* World = GetWorld();
	if (!World || CurrentGameStateInternal != ECGS::InGame)
	{
		return;
	}

	InGameTimerSecRemainInternal -= UGameStateDataAsset::Get().GetTickInterval();

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

// Is called during the In-Game state to try to register the End-Game state
void AMyGameStateBase::UpdateEndGameStates()
{
	if (!DoesWantUpdateEndState())
	{
		return;
	}

	bWantsUpdateEndStateInternal = false;

	for (APlayerState* PlayerStateIt : PlayerArray)
	{
		AMyPlayerState* MyPlayerState = PlayerStateIt ? Cast<AMyPlayerState>(PlayerStateIt) : nullptr;
		if (!MyPlayerState
		    || MyPlayerState->GetEndGameState() != EEndGameState::None) // Already set the state
		{
			continue;
		}

		MyPlayerState->UpdateEndGameState();
	}

	if (UMyBlueprintFunctionLibrary::GetAlivePlayersNum() <= 1)
	{
		ServerSetGameState(ECGS::EndGame);
	}
}

// Called when any player or bot was exploded
void AMyGameStateBase::OnAnyCharacterDestroyed()
{
	bWantsUpdateEndStateInternal = true;
}

// Enables or disable all game features
void AMyGameStateBase::SetGameFeaturesEnabled(bool bEnable)
{
	UGameFeaturesSubsystem& GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();
	const TArray<FName>& GameFeaturesToEnable = UGameStateDataAsset::Get().GetGameFeaturesToEnable();
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
