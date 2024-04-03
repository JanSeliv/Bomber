// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMSpotsSubsystem.h"
//---
#include "NMMUtils.h"
#include "Components/NMMSpotComponent.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMSpotsSubsystem)

UNMMSpotsSubsystem& UNMMSpotsSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNMMSpotsSubsystem* ThisSubsystem = UNMMUtils::GetSpotsSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'NMMSpotsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns true if any Main-Menu spot is fully initialized: spawned on the level and finished loading its Master Sequence
bool UNMMSpotsSubsystem::IsAnyMainMenuSpotReady() const
{
	return MainMenuSpotsInternal.ContainsByPredicate([](const UNMMSpotComponent* MainMenuSpotComponent)
	{
		return MainMenuSpotComponent && MainMenuSpotComponent->GetMasterPlayer() != nullptr;
	});
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

	// If instant, then switch to the next spot, it will possess the camera and start playing its cinematic
	// Otherwise start transition to the next spot
	const bool bInstant = UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled();
	UNMMBaseSubsystem::Get().SetNewMainMenuState(bInstant ? ENMMState::Idle : ENMMState::Transition);

	return NextMainMenuSpot;
}

// Clears all transient data contained in this subsystem
void UNMMSpotsSubsystem::Deinitialize()
{
	MainMenuSpotsInternal.Empty();

	Super::Deinitialize();
}
