// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMSpotsSubsystem.h"
//---
#include "NMMUtils.h"
#include "Components/NMMSpotComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSpotsSubsystem)

UNMMSpotsSubsystem& UNMMSpotsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNMMSpotsSubsystem* ThisSubsystem = UNMMUtils::GetSpotsSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'NMMSpotsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns true if any Main-Menu spot is fully initialized: spawned on the level and finished loading its Master Sequence
bool UNMMSpotsSubsystem::IsActiveMenuSpotReady() const
{
	const UNMMSpotComponent* ActiveSpot = GetCurrentSpot();
	return ActiveSpot && ActiveSpot->GetMasterPlayer() != nullptr;
}

// Add new Main-Menu spot, so it can be obtained by other objects
void UNMMSpotsSubsystem::AddNewMainMenuSpot(UNMMSpotComponent* NewMainMenuSpotComponent)
{
	if (ensureMsgf(NewMainMenuSpotComponent, TEXT("%s: 'NewMainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		MainMenuSpotsInternal.AddUnique(NewMainMenuSpotComponent);
	}
}

// Removes Main-Menu spot if should not be available by other objects anymore
void UNMMSpotsSubsystem::RemoveMainMenuSpot(UNMMSpotComponent* MainMenuSpotComponent)
{
	if (ensureMsgf(MainMenuSpotComponent, TEXT("%s: 'MainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		MainMenuSpotsInternal.RemoveSwap(MainMenuSpotComponent);
	}
}

// Returns currently selected Main-Menu spot
UNMMSpotComponent* UNMMSpotsSubsystem::GetCurrentSpot() const
{
	for (UNMMSpotComponent* MainMenuSpotComponent : MainMenuSpotsInternal)
	{
		if (MainMenuSpotComponent && MainMenuSpotComponent->GetCinematicRow().RowIndex == ActiveMenuSpotIdxInternal)
		{
			return MainMenuSpotComponent;
		}
	}

	return nullptr;
}

// Returns Main-Menu spots by given level type
void UNMMSpotsSubsystem::GetMainMenuSpotsByLevelType(TArray<UNMMSpotComponent*>& OutSpots, ELevelType LevelType) const
{
	for (UNMMSpotComponent* MainMenuSpotComponent : MainMenuSpotsInternal)
	{
		if (MainMenuSpotComponent
			&& MainMenuSpotComponent->GetCinematicRow().LevelType == LevelType)
		{
			OutSpots.AddUnique(MainMenuSpotComponent);
		}
	}

	// Sort the array based on the RowIndex
	OutSpots.Sort([](const UNMMSpotComponent& A, const UNMMSpotComponent& B)
	{
		return A.GetCinematicRow().RowIndex < B.GetCinematicRow().RowIndex;
	});
}

// Returns next or previous Main-Menu spot by given incrementer
UNMMSpotComponent* UNMMSpotsSubsystem::GetNextSpot(int32 Incrementer, ELevelType LevelType) const
{
	TArray<UNMMSpotComponent*> CurrentLevelTypeSpots;
	GetMainMenuSpotsByLevelType(/*out*/CurrentLevelTypeSpots, LevelType);

	// Extract the row indices, so we can track the bounds
	TArray<int32> SpotRowIndices;
	for (const UNMMSpotComponent* SpotIt : CurrentLevelTypeSpots)
	{
		SpotRowIndices.AddUnique(SpotIt->GetCinematicRow().RowIndex);
	}

	const bool bFoundActiveIdx = SpotRowIndices.Contains(ActiveMenuSpotIdxInternal);
	if (!ensureMsgf(bFoundActiveIdx, TEXT("%s: 'ActiveMenuSpotIdxInternal' is not found in the 'SpotRowIndices'"), *FString(__FUNCTION__)))
	{
		// Most likely the level is switched that could be not supported yet
		return nullptr;
	}

	// Find the new index based on the incrementer
	// If there is no next spot in array, it will take the first one with its index and vise versa for decrementing
	const int32 ActiveSpotPosition = SpotRowIndices.IndexOfByKey(ActiveMenuSpotIdxInternal);
	const int32 NewSpotIndex = (ActiveSpotPosition + Incrementer + SpotRowIndices.Num()) % SpotRowIndices.Num();
	checkf(CurrentLevelTypeSpots.IsValidIndex(NewSpotIndex), TEXT("ERROR: [%i] %s:\n'CurrentLevelTypeSpots array has to have NewSpotIndex since it's the same size as SpotRowIndices array!"), __LINE__, *FString(__FUNCTION__));
	return CurrentLevelTypeSpots[NewSpotIndex];
}

// Goes to another Spot to show another player character on current level
UNMMSpotComponent* UNMMSpotsSubsystem::MoveMainMenuSpot(int32 Incrementer)
{
	UNMMSpotComponent* NextMainMenuSpot = GetNextSpot(Incrementer, UMyBlueprintFunctionLibrary::GetLevelType());
	if (!ensureMsgf(NextMainMenuSpot, TEXT("ASSERT: [%i] %s:\n'NextMainMenuSpot' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return nullptr;
	}

	// Set the new active spot index
	ActiveMenuSpotIdxInternal = NextMainMenuSpot->GetCinematicRow().RowIndex;
	LastMoveSpotDirectionInternal = Incrementer;

	// If transition happened in opened menu, change the internal state 
	if (AMyGameStateBase::GetCurrentGameState() == ECGS::Menu)
	{
		// If instant, then switch to the next spot, it will possess the camera and start playing its cinematic
		// Otherwise start transition to the next spot
		const bool bInstant = UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled();
		UNMMBaseSubsystem::Get().SetNewMainMenuState(bInstant ? ENMMState::Idle : ENMMState::Transition);
	}
	else // In-game
	{
		// Thw spot switch happened during the game, just update the mesh hiddenly
		NextMainMenuSpot->ApplyMeshOnPlayer();
	}

	return NextMainMenuSpot;
}

// Is code alternative to MoveMainMenuSpot, but allows to use custom predicate
UNMMSpotComponent* UNMMSpotsSubsystem::MoveMainMenuSpotByPredicate(int32 Incrementer, const TFunctionRef<bool(UNMMSpotComponent*)>& Predicate)
{
	const int32 FinalIncrementer = [&]() -> int32
	{
		const ELevelType LevelType = UMyBlueprintFunctionLibrary::GetLevelType();
		TArray<UNMMSpotComponent*> LevelTypeSpots;
		GetMainMenuSpotsByLevelType(LevelTypeSpots, LevelType);

		if (LevelTypeSpots.IsEmpty())
		{
			return 0;
		}

		UNMMSpotComponent* Spot = nullptr;
		int32 Attempts = 0;
		const int32 MaxAttempts = LevelTypeSpots.Num();
		int32 NewIncrementer = Incrementer;

		while (Attempts < MaxAttempts)
		{
			Spot = GetNextSpot(NewIncrementer, LevelType);
			if (Spot && Predicate(Spot))
			{
				return NewIncrementer;
			}

			NewIncrementer += Incrementer;
			Attempts++;
		}

		return 0;
	}();

	// -1 or 1 to move left or right, 0 if no spot found
	if (FinalIncrementer == 0)
	{
		return nullptr;
	}

	// Move to the new spot
	return MoveMainMenuSpot(FinalIncrementer);
}

// Attempts to switch to the active menu spot if current slot is not available for any reason
void UNMMSpotsSubsystem::HandleUnavailableMenuSpot()
{
	const UNMMSpotComponent* CurrentSpot = GetCurrentSpot();
	if (CurrentSpot && CurrentSpot->IsSpotAvailable())
	{
		// No need to switch, the current spot is valid
		return;
	}

	// Current is inactive (hidden), likely it's locked by different systems
	// Switch to any previous spot that is active
	constexpr int32 BackwardDir = -1;
	MoveMainMenuSpotByPredicate(BackwardDir, [](const UNMMSpotComponent* Spot) { return Spot->IsSpotAvailable(); });
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

void UNMMSpotsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Listen Main Menu states
	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(this, &ThisClass::OnNewMainMenuStateChanged);
	if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)
	{
		// State is already set, apply it
		OnNewMainMenuStateChanged(BaseSubsystem.GetCurrentMenuState());
	}

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Clears all transient data contained in this subsystem
void UNMMSpotsSubsystem::Deinitialize()
{
	MainMenuSpotsInternal.Empty();

	Super::Deinitialize();
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the Main Menu state was changed
void UNMMSpotsSubsystem::OnNewMainMenuStateChanged_Implementation(ENMMState NewState)
{
	if (NewState == ENMMState::None)
	{
		LastMoveSpotDirectionInternal = 0;
	}
}

// Called when the current game state was changed
void UNMMSpotsSubsystem::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
	case ECGS::GameStarting:
		HandleUnavailableMenuSpot();
		break;

	default: break;
	}
}
