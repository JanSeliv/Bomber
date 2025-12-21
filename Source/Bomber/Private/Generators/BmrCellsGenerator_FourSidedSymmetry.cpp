// Copyright (c) Yevhenii Selivanov

#include "Generators/BmrCellsGenerator_FourSidedSymmetry.h"

// Bomber
#include "Bomber.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "Containers/Queue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCellsGenerator_FourSidedSymmetry)

// Is overriden to implement custom level generation logic
TMap<FBmrCell, EBmrActorType> UBmrCellsGenerator_FourSidedSymmetry::GenerateLevel(FBmrGeneratorData&& GeneratorData)
{
	if (!GeneratorData.IsValid())
	{
		return Super::GenerateLevel(MoveTemp(GeneratorData));
	}

	// --- Step 1: Initial Setup and Projection ---
	// Defines the top-left quarter and projects any dragged powerups/boxes into that quarter
	// so they can be included in the pathfinding logic.
	const FBmrCellsArr QuarterCellsArray = FBmrCell::GetTopLeftQuarterOnGrid(GeneratorData.AllCells, GeneratorData.MapScale);
	const FBmrCells QuarterCellsSet(QuarterCellsArray);

	FBmrCells PriorityTargets, DraggedWalls;
	const int32 MaxIndexX = GeneratorData.MapScale.X - 1;
	const int32 MaxIndexY = GeneratorData.MapScale.Y - 1;

	for (const TTuple<FBmrCell, EBmrActorType>& DraggedPair : GeneratorData.DraggedCells)
	{
		if (DraggedPair.Value == EBmrActorType::Wall)
		{
			DraggedWalls.Add(DraggedPair.Key);
			continue;
		}
		if (DraggedPair.Value == EBmrActorType::Powerup || DraggedPair.Value == EBmrActorType::Box)
		{
			const FIntPoint OriginalPosition = FBmrCell::GetPositionByCellOnGrid(DraggedPair.Key, GeneratorData.AllCells, GeneratorData.MapScale.X);
			const int32 ProjectedX = FMath::Min(OriginalPosition.X, MaxIndexX - OriginalPosition.X);
			const int32 ProjectedY = FMath::Min(OriginalPosition.Y, MaxIndexY - OriginalPosition.Y);
			const FBmrCell ProjectedCell = FBmrCell::GetCellByPositionOnGrid({ProjectedX, ProjectedY}, GeneratorData.AllCells, GeneratorData.MapScale.X);
			if (ProjectedCell.IsValid())
			{
				PriorityTargets.Add(ProjectedCell);
			}
		}
	}

	// --- Step 2: Generate Core Path Network ---
	// Creates a network of empty cells within the quarter that connects the player start,
	// all priority targets, and the right and bottom borders.
	static constexpr int32 HalfAxisScaleDivider = 2;
	const FIntPoint QuarterScale(GeneratorData.MapScale.X / HalfAxisScaleDivider + 1, GeneratorData.MapScale.Y / HalfAxisScaleDivider + 1);
	const FBmrCell PlayerStartCell = QuarterCellsArray.IsEmpty() ? FBmrCell::InvalidCell : QuarterCellsArray[0];

	const FBmrGeneratorQuarterPathParams PathParams{QuarterCellsSet, PlayerStartCell, QuarterScale, DraggedWalls, PriorityTargets};
	FBmrCells PathCells = GeneratePathsForQuarter(PathParams, GeneratorData); // Made non-const to allow moving

	// --- Step 3: Fill Quarter with Obstacles ---
	// Places indestructible walls and destructible boxes in the remaining empty space,
	// based on the fill percentages defined in the Generation Settings.
	TMap<FBmrCell, EBmrActorType> QuarterActors = FillObstaclesInQuarter({MoveTemp(PathCells), PlayerStartCell}, PathParams, GeneratorData); // Use MoveTemp

	// --- Step 4: Apply Symmetry ---
	// Mirrors the generated quarter to the other three corners of the map to create
	// the final, full level layout.
	TMap<FBmrCell, EBmrActorType> ActorsToSpawn = ApplySymmetry(MoveTemp(QuarterActors), GeneratorData);

	// --- Step 5: Finalize with Dragged Actors ---
	// Merges the designer-placed actors, overwriting any generated actors that
	// occupy the same cells.
	ActorsToSpawn.Append(MoveTemp(GeneratorData.DraggedCells));

	return ActorsToSpawn;
}

// Creates a network of paths from the player start to priority targets and quarter borders.
FBmrCells UBmrCellsGenerator_FourSidedSymmetry::GeneratePathsForQuarter(const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData)
{
	/**
	 * Example Path Network ('P'rimary, 'R'ight, 'B'ottom):
	 *   A   B   C   D   E
	 * \+---+---+---+---+---+
	 * 1 | S | P | P | . | . | (S = Start Cell)
	 * \+---+---+---+---+---+
	 * 2 | B | B | P | . | . |
	 * \+---+---+---+---+---+
	 * 3 | . | B | P | R | R | <- Path reaches right border
	 * \+---+---+---+---+---+
	 * 4 | . | B | . | . | . |
	 * \+---+---+---+---+---+
	 * 5 | . | B | B | . | . | <- Path reaches bottom border
	 * \+---+---+---+---+---+
	 */
	if (!PathParams.StartCell.IsValid())
	{
		return FBmrCells();
	}

	FBmrCells PathNetwork;
	PathNetwork.Add(PathParams.StartCell);
	FBmrCellsArr PrimaryPath, PathToRight, PathToBottom;

	static const FIntPoint Right(1, 0), Left(-1, 0), Down(0, 1), Up(0, -1);
	static const TArray HorizontalAndUpDirs = {Right, Left, Up};

	for (const FBmrCell& TargetCell : PathParams.PriorityTargets)
	{
		if (PathNetwork.Contains(TargetCell))
			continue;
		const FBmrCellsArr NetworkAsArray = PathNetwork.Array();
		const FBmrCell RandomStart = NetworkAsArray[FMath::RandRange(0, NetworkAsArray.Num() - 1)];
		const FBmrGeneratorPathfindParams FindParams{RandomStart, {TargetCell}, Down, HorizontalAndUpDirs};
		const FBmrCellsArr PathToTarget = FindDirectedPathWithDetours(FindParams, PathParams, GeneratorData);
		PrimaryPath.Append(PathToTarget);
		PathNetwork.Append(PathToTarget);
	}

	const int32 RightBorderIndex = PathParams.QuarterScale.X - 1;
	FBmrCells RightBorderCells;
	for (int32 Index = 0; Index < PathParams.QuarterScale.Y; ++Index)
	{
		RightBorderCells.Add(FBmrCell::GetCellByPositionOnGrid({RightBorderIndex, Index}, GeneratorData.AllCells, GeneratorData.MapScale.X));
	}
	if (!RightBorderCells.IsEmpty())
	{
		const FBmrCellsArr NetworkAsArray = PathNetwork.Array();
		const FBmrCell RandomStart = NetworkAsArray[FMath::RandRange(0, NetworkAsArray.Num() - 1)];
		const FBmrGeneratorPathfindParams FindParams{RandomStart, RightBorderCells, Right, {Up, Down}};
		PathToRight = FindDirectedPathWithDetours(FindParams, PathParams, GeneratorData);
		PathNetwork.Append(PathToRight);
	}

	const int32 BottomBorderIndex = PathParams.QuarterScale.Y - 1;
	FBmrCells BottomBorderCells;
	for (int32 Index = 0; Index < PathParams.QuarterScale.X; ++Index)
	{
		BottomBorderCells.Add(FBmrCell::GetCellByPositionOnGrid({Index, BottomBorderIndex}, GeneratorData.AllCells, GeneratorData.MapScale.X));
	}
	if (!BottomBorderCells.IsEmpty())
	{
		const FBmrCellsArr NetworkAsArray = PathNetwork.Array();
		const FBmrCell RandomStart = NetworkAsArray[FMath::RandRange(0, NetworkAsArray.Num() - 1)];
		const FBmrGeneratorPathfindParams FindParams{RandomStart, BottomBorderCells, Down, {Left, Right}};
		PathToBottom = FindDirectedPathWithDetours(FindParams, PathParams, GeneratorData);
		PathNetwork.Append(PathToBottom);
	}

#if !UE_BUILD_SHIPPING
	if (bDisplayPrimaryPath)
	{
		FBmrDisplayCellsParams DisplayParams;
		DisplayParams.bClearPreviousDisplays = true;
		DisplayParams.TextColor = FLinearColor::White;
		UBmrCellUtilsLibrary::DisplayCells(this, FBmrCells(PrimaryPath), DisplayParams);
	}
	if (bDisplayRightPath)
	{
		FBmrDisplayCellsParams DisplayParams;
		DisplayParams.bClearPreviousDisplays = !bDisplayPrimaryPath;
		DisplayParams.TextColor = FLinearColor::Blue;
		DisplayParams.RenderString = TEXT("->");
		UBmrCellUtilsLibrary::DisplayCells(this, FBmrCells(PathToRight), DisplayParams);
	}
	if (bDisplayBottomPath)
	{
		FBmrDisplayCellsParams DisplayParams;
		DisplayParams.bClearPreviousDisplays = !bDisplayPrimaryPath && !bDisplayRightPath;
		DisplayParams.TextColor = FLinearColor::Green;
		DisplayParams.RenderString = TEXT("v");
		UBmrCellUtilsLibrary::DisplayCells(this, FBmrCells(PathToBottom), DisplayParams);
	}
#endif

	return PathNetwork;
}

// Fills the quarter with walls and boxes based on fill percentages
TMap<FBmrCell, EBmrActorType> UBmrCellsGenerator_FourSidedSymmetry::FillObstaclesInQuarter(FBmrGeneratorQuarterFillParams&& FillParams, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData)
{
	/**
	 * Example Quarter Fill:
	 * +---+---+---+---+
	 * | P |   | █ | □ | (P=Player, █=Wall, □=Box)
	 * +---+---+---+---+
	 * |   |   | □ | █ | (Empty cells are part of the path network)
	 * +---+---+---+---+
	 */
	TMap<FBmrCell, EBmrActorType> QuarterActors;
	if (!FillParams.PlayerStartCell.IsValid())
	{
		return QuarterActors;
	}

	QuarterActors.Emplace(FillParams.PlayerStartCell, EBmrActorType::Player);
	const FIntPoint PlayerPosition = FBmrCell::GetPositionByCellOnGrid(FillParams.PlayerStartCell, GeneratorData.AllCells, GeneratorData.MapScale.X);

	FBmrCellsArr PossibleWallLocations;
	for (const FBmrCell& CurrentCell : PathParams.QuarterCells)
	{
		if (FillParams.PathCells.Contains(CurrentCell))
		{
			continue;
		}
		const FIntPoint CellPosition = FBmrCell::GetPositionByCellOnGrid(CurrentCell, GeneratorData.AllCells, GeneratorData.MapScale.X);
		if (FMath::Abs(CellPosition.X - PlayerPosition.X) + FMath::Abs(CellPosition.Y - PlayerPosition.Y) <= 1)
		{
			continue;
		}
		PossibleWallLocations.Add(CurrentCell);
	}

	static constexpr float PercentageDivider = 100.0f;
	const int32 NumWallsToPlace = FMath::FloorToInt(PossibleWallLocations.Num() * (GeneratorData.GenerationSettings.WallsChance / PercentageDivider));
	FBmrCell::RandShuffle(PossibleWallLocations);

	FBmrCells PlacedWallCells;
	for (int32 Index = 0; Index < NumWallsToPlace; ++Index)
	{
		const FBmrCell& WallCell = PossibleWallLocations[Index];
		QuarterActors.Emplace(WallCell, EBmrActorType::Wall);
		PlacedWallCells.Add(WallCell);
	}

	const FBmrCells WalkableArea = FindWalkableArea(FillParams.PlayerStartCell, PlacedWallCells, PathParams, GeneratorData);
	FBmrCellsArr PossibleBoxLocations;
	for (const FBmrCell& CurrentCell : WalkableArea)
	{
		if (QuarterActors.Contains(CurrentCell))
		{
			continue;
		}
		const FIntPoint CellPosition = FBmrCell::GetPositionByCellOnGrid(CurrentCell, GeneratorData.AllCells, GeneratorData.MapScale.X);
		if (FMath::Abs(CellPosition.X - PlayerPosition.X) + FMath::Abs(CellPosition.Y - PlayerPosition.Y) <= 1)
		{
			continue;
		}
		PossibleBoxLocations.Add(CurrentCell);
	}

	const int32 NumBoxesToPlace = FMath::FloorToInt(PossibleBoxLocations.Num() * (GeneratorData.GenerationSettings.BoxesChance / PercentageDivider));
	FBmrCell::RandShuffle(PossibleBoxLocations);

	for (int32 Index = 0; Index < NumBoxesToPlace; ++Index)
	{
		QuarterActors.Emplace(PossibleBoxLocations[Index], EBmrActorType::Box);
	}
	return QuarterActors;
}

// Applies four-sided symmetry to the generated quarter map to build the full level.
TMap<FBmrCell, EBmrActorType> UBmrCellsGenerator_FourSidedSymmetry::ApplySymmetry(TMap<FBmrCell, EBmrActorType>&& QuarterActors, const FBmrGeneratorData& GeneratorData)
{
	/**
	 * Quarter:       Full Map:
	 * P □ █          P □ █ █ □ P
	 *   + .            + . . + .
	 *                . . . . . .
	 *                . . . . . .
	 *                . + . . + .
	 *                P □ █ █ □ P
	 */
	TMap<FBmrCell, EBmrActorType> FullMapActors;
	const int32 MaxIndexX = GeneratorData.MapScale.X - 1;
	const int32 MaxIndexY = GeneratorData.MapScale.Y - 1;

	for (const TTuple<FBmrCell, EBmrActorType>& ActorEntry : QuarterActors)
	{
		const FBmrCell& QuarterCell = ActorEntry.Key;
		const EBmrActorType ActorType = ActorEntry.Value;
		const FIntPoint OriginalPosition = FBmrCell::GetPositionByCellOnGrid(QuarterCell, GeneratorData.AllCells, GeneratorData.MapScale.X);
		const int32 SymmetricalX = MaxIndexX - OriginalPosition.X;
		const int32 SymmetricalY = MaxIndexY - OriginalPosition.Y;

		const TArray<FIntPoint> PositionsToPlace = {
		    {OriginalPosition.X, OriginalPosition.Y}, {SymmetricalX, OriginalPosition.Y},
		    {OriginalPosition.X, SymmetricalY}, {SymmetricalX, SymmetricalY}};

		for (const FIntPoint& SymPosition : PositionsToPlace)
		{
			const FBmrCell TargetCell = FBmrCell::GetCellByPositionOnGrid(SymPosition, GeneratorData.AllCells, GeneratorData.MapScale.X);
			if (TargetCell.IsValid() && !FullMapActors.Contains(TargetCell))
			{
				FullMapActors.Emplace(TargetCell, ActorType);
			}
		}
	}
	return FullMapActors;
}

// Creates a directed but randomized path from a start cell to one of the target cells
FBmrCellsArr UBmrCellsGenerator_FourSidedSymmetry::FindDirectedPathWithDetours(const FBmrGeneratorPathfindParams& Params, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData)
{
	/**
	 * This function uses a biased random walk algorithm. For example, to find a path
	 * from Start (S) to End (E), it will primarily move right but can take random (up, down)
	 * detours up or down to create a more natural-looking path.
	 *
	 * Example Path ('+'):
	 * S + . . . .
	 * . + . + + E
	 * . + + + . .
	 * . . . . . .
	 * . . . . . .
	 */
	FBmrCellsArr PathStack;
	if (!Params.StartNode.IsValid())
	{
		return PathStack;
	}
	PathStack.Push(Params.StartNode);

	FBmrCells VisitedCells = PathParams.DraggedWalls;
	VisitedCells.Add(Params.StartNode);

	static constexpr int32 PrimaryMoveBias = 3;
	FBmrCellsArr PossibleNextCells;

	while (!PathStack.IsEmpty())
	{
		const FBmrCell CurrentCell = PathStack.Last();
		if (Params.TargetCells.Contains(CurrentCell))
		{
			return PathStack;
		}

		PossibleNextCells.Reset();
		const FIntPoint CurrentPosition = FBmrCell::GetPositionByCellOnGrid(CurrentCell, GeneratorData.AllCells, GeneratorData.MapScale.X);

		auto CheckNeighbor = [&](const FIntPoint& NeighborPos, bool bIsPrimary)
		{
			// Constrain the path to within the quarter's dimensions
			if (NeighborPos.X < PathParams.QuarterScale.X && NeighborPos.Y < PathParams.QuarterScale.Y && NeighborPos.X >= 0 && NeighborPos.Y >= 0)
			{
				const FBmrCell NeighborCell = FBmrCell::GetCellByPositionOnGrid(NeighborPos, GeneratorData.AllCells, GeneratorData.MapScale.X);
				if (NeighborCell.IsValid() && !VisitedCells.Contains(NeighborCell))
				{
					const int32 NumToAdd = bIsPrimary ? PrimaryMoveBias : 1;
					for (int32 i = 0; i < NumToAdd; ++i)
					{
						PossibleNextCells.Add(NeighborCell);
					}
				}
			}
		};

		CheckNeighbor(CurrentPosition + Params.PrimaryDirection, true);
		for (const FIntPoint& SecondaryDir : Params.SecondaryDirections)
		{
			CheckNeighbor(CurrentPosition + SecondaryDir, false);
		}

		if (!PossibleNextCells.IsEmpty())
		{
			const FBmrCell NextCell = PossibleNextCells[FMath::RandRange(0, PossibleNextCells.Num() - 1)];
			VisitedCells.Add(NextCell);
			PathStack.Push(NextCell);
		}
		else
		{
			PathStack.Pop();
		}
	}
	return {Params.StartNode};
}

// Finds all reachable cells from a starting point using Breadth-First Search
FBmrCells UBmrCellsGenerator_FourSidedSymmetry::FindWalkableArea(const FBmrCell& StartCell, const FBmrCells& WallCells, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData)
{
	/**
	 * This function performs a Breadth-First Search (a flood-fill) to discover every
	 * cell that can be reached from a given start point without crossing over a set of specified wall cells.
	 */
	FBmrCells WalkableArea;
	if (!StartCell.IsValid())
	{
		return WalkableArea;
	}

	TQueue<FBmrCell> CellsToVisit;
	FBmrCells VisitedCells = WallCells;
	CellsToVisit.Enqueue(StartCell);
	VisitedCells.Add(StartCell);

	while (!CellsToVisit.IsEmpty())
	{
		FBmrCell CurrentCell;
		CellsToVisit.Dequeue(CurrentCell);
		WalkableArea.Add(CurrentCell);

		const FIntPoint CurrentPosition = FBmrCell::GetPositionByCellOnGrid(CurrentCell, GeneratorData.AllCells, GeneratorData.MapScale.X);
		static const TArray<FIntPoint> AllDirections = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

		for (const FIntPoint& Direction : AllDirections)
		{
			const FIntPoint NeighborPosition = CurrentPosition + Direction;
			// Constrain the flood-fill to within the quarter's dimensions
			if (NeighborPosition.X < PathParams.QuarterScale.X && NeighborPosition.Y < PathParams.QuarterScale.Y && NeighborPosition.X >= 0 && NeighborPosition.Y >= 0)
			{
				const FBmrCell NeighborCell = FBmrCell::GetCellByPositionOnGrid(NeighborPosition, GeneratorData.AllCells, GeneratorData.MapScale.X);
				if (NeighborCell.IsValid() && !VisitedCells.Contains(NeighborCell))
				{
					VisitedCells.Add(NeighborCell);
					CellsToVisit.Enqueue(NeighborCell);
				}
			}
		}
	}
	return WalkableArea;
}