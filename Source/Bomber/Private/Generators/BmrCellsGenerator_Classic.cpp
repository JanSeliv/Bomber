// Copyright (c) Yevhenii Selivanov

#include "Generators/BmrCellsGenerator_Classic.h"

// Bomber
#include "Bomber.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCellsGenerator_Classic)

// Is overriden to implement custom level generation logic
TMap<FBmrCell, EBmrActorType> UBmrCellsGenerator_Classic::GenerateLevel(FBmrGeneratorData&& GeneratorData)
{
	if (!GeneratorData.IsValid())
	{
		// Pass the original params struct to the super call for the fallback
		return Super::GenerateLevel(MoveTemp(GeneratorData));
	}

	TMap<FBmrCell, EBmrActorType> ActorsToSpawn;
	FBmrCellsArr PossibleBoxCells;

	// --- Define the safe zones around all four player corners ---
	TSet<FIntPoint> SafeZoneCoordinates;
	const int32 MaxX = GeneratorData.MapScale.X - 1;
	const int32 MaxY = GeneratorData.MapScale.Y - 1;
	SafeZoneCoordinates.Append({// Top-Left Player
	    {0, 0}, {1, 0}, {0, 1},
	    // Top-Right Player
	    {MaxX, 0}, {MaxX - 1, 0}, {MaxX, 1},
	    // Bottom-Left Player
	    {0, MaxY}, {1, MaxY}, {0, MaxY - 1},
	    // Bottom-Right Player
	    {MaxX, MaxY}, {MaxX - 1, MaxY}, {MaxX, MaxY - 1}});

	// --- First Pass: Place deterministic walls and gather possible box locations ---
	for (int32 PosY = 0; PosY < GeneratorData.MapScale.Y; ++PosY)
	{
		for (int32 PosX = 0; PosX < GeneratorData.MapScale.X; ++PosX)
		{
			const FIntPoint CurrentPosition(PosX, PosY);
			if (SafeZoneCoordinates.Contains(CurrentPosition))
			{
				continue;
			}

			const bool bIsUnevenX = PosX % 2 != 0;
			const bool bIsUnevenY = PosY % 2 != 0;
			const FBmrCell TargetCell = FBmrCell::GetCellByPositionOnGrid(CurrentPosition, GeneratorData.AllCells, GeneratorData.MapScale.X);

			if (bIsUnevenX && bIsUnevenY)
			{
				if (TargetCell.IsValid())
				{
					ActorsToSpawn.Emplace(TargetCell, EBmrActorType::Wall);
				}
			}
			else // All other non-safe cells are candidates for destructible boxes
			{
				if (TargetCell.IsValid())
				{
					PossibleBoxCells.Add(TargetCell);
				}
			}
		}
	}

	// --- Second Pass: Randomly place a percentage of boxes from the candidate list ---
	static constexpr float PercentageDivider = 100.0f;
	const float BoxFillPercentage = GeneratorData.GenerationSettings.BoxesChance / PercentageDivider;
	const int32 NumBoxesToPlace = FMath::FloorToInt(PossibleBoxCells.Num() * BoxFillPercentage);
	for (int32 Index = 0; Index < NumBoxesToPlace; ++Index)
	{
		if (PossibleBoxCells.IsEmpty())
		{
			break;
		}

		const int32 RandomIndex = FMath::RandRange(0, PossibleBoxCells.Num() - 1);
		const FBmrCell BoxCell = PossibleBoxCells[RandomIndex];
		ActorsToSpawn.Emplace(BoxCell, EBmrActorType::Box);
		PossibleBoxCells.RemoveAtSwap(RandomIndex);
	}

	// --- Final Pass: Place players in the four corners ---
	const TArray<FIntPoint> CornerPositions = {
	    {0, 0}, {MaxX, 0}, // Top-Left, Top-Right
	    {0, MaxY}, {MaxX, MaxY} // Bottom-Left, Bottom-Right
	};

	for (const FIntPoint& CornerPos : CornerPositions)
	{
		const FBmrCell CornerCell = FBmrCell::GetCellByPositionOnGrid(CornerPos, GeneratorData.AllCells, GeneratorData.MapScale.X);
		if (CornerCell.IsValid())
		{
			ActorsToSpawn.Emplace(CornerCell, EBmrActorType::Player);
		}
	}

	// Always respect user-dragged cells, moving them in to overwrite generated actors
	ActorsToSpawn.Append(MoveTemp(GeneratorData.DraggedCells));

	return ActorsToSpawn;
}