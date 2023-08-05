// Copyright (c) Yevhenii Selivanov

#include "Data/NewMainMenuSubsystem.h"
//---
#include "Components/NewMainMenuSpotComponent.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNewMainMenuSubsystem& UNewMainMenuSubsystem::Get()
{
	const UWorld* World = UMyBlueprintFunctionLibrary::GetStaticWorld();
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UNewMainMenuSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Add new Main-Menu spot, so it can be obtained by other objects
void UNewMainMenuSubsystem::AddNewMainMenuSpot(UNewMainMenuSpotComponent* NewMainMenuSpotComponent)
{
	if (ensureMsgf(NewMainMenuSpotComponent, TEXT("%s: 'NewMainMenuSpotComponent' is null"), *FString(__FUNCTION__)))
	{
		MainMenuSpotsInternal.AddUnique(NewMainMenuSpotComponent);
	}
}

// Returns currently selected Main-Menu spot
UNewMainMenuSpotComponent* UNewMainMenuSubsystem::GetActiveMainMenuSpotComponent() const
{
	for (UNewMainMenuSpotComponent* MainMenuSpotComponent : MainMenuSpotsInternal)
	{
		if (MainMenuSpotComponent && MainMenuSpotComponent->GetCinematicRow().RowIndex == ActiveMainMenuSpotIdx)
		{
			return MainMenuSpotComponent;
		}
	}

	return nullptr;
}

// Returns Main-Menu spots by given level type
void UNewMainMenuSubsystem::GetMainMenuSpotsByLevelType(TArray<UNewMainMenuSpotComponent*>& OutSpots, ELevelType LevelType) const
{
	for (UNewMainMenuSpotComponent* MainMenuSpotComponent : MainMenuSpotsInternal)
	{
		if (MainMenuSpotComponent
			&& MainMenuSpotComponent->GetCinematicRow().LevelType == LevelType)
		{
			OutSpots.AddUnique(MainMenuSpotComponent);
		}
	}

	// Sort the array based on the RowIndex
	OutSpots.Sort([](const UNewMainMenuSpotComponent& A, const UNewMainMenuSpotComponent& B)
	{
		return A.GetCinematicRow().RowIndex < B.GetCinematicRow().RowIndex;
	});
}

// Goes to another Spot to show another player character on current level
UNewMainMenuSpotComponent* UNewMainMenuSubsystem::MoveMainMenuSpot(int32 Incrementer)
{
	// Get all rows of current level type
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	TArray<UNewMainMenuSpotComponent*> CurrentLevelTypeSpots;
	GetMainMenuSpotsByLevelType(/*out*/CurrentLevelTypeSpots, CurrentLevelType);

	// Extract the row indices, so we can track the bounds
	TArray<int32> SpotRowIndices;
	for (const UNewMainMenuSpotComponent* SpotIt : CurrentLevelTypeSpots)
	{
		SpotRowIndices.AddUnique(SpotIt->GetCinematicRow().RowIndex);
	}

	const bool bFoundActiveIdx = SpotRowIndices.Contains(ActiveMainMenuSpotIdx);
	if (!ensureMsgf(bFoundActiveIdx, TEXT("%s: 'ActiveMainMenuSpotIdx' is not found in the 'SpotRowIndices'"), *FString(__FUNCTION__)))
	{
		// Most likely the level is switched that could be not supported yet
		return nullptr;
	}

	// Stop the current spot
	const int32 ActiveSpotPosition = SpotRowIndices.IndexOfByKey(ActiveMainMenuSpotIdx);
	UNewMainMenuSpotComponent* CurrentSpot = CurrentLevelTypeSpots[ActiveSpotPosition];
	check(CurrentSpot);
	CurrentSpot->StopMasterSequence();

	// Find the new index based on the incrementer
	// If there is no next spot in array, it will take the first one with its index and vise versa for decrementing
	const int32 NewSpotIndex = (ActiveSpotPosition + Incrementer + SpotRowIndices.Num()) % SpotRowIndices.Num();
	ActiveMainMenuSpotIdx = SpotRowIndices[NewSpotIndex];

	// Play the new spot
	UNewMainMenuSpotComponent* NewSpot = CurrentLevelTypeSpots[NewSpotIndex];
	NewSpot->PlayIdlePart();

	return NewSpot;
}
