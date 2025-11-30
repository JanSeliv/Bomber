// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// Bomber
#include "Actors/BmrBombAbilityActor.h"
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "GameFramework/BmrCheatManager.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"

// UE
#include "Components/TextRenderComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCellUtilsLibrary)

// Default params to display cells
const FBmrDisplayCellsParams FBmrDisplayCellsParams::EmptyParams = FBmrDisplayCellsParams();

// Creates 'Break Cell' node with X, Y, Z outputs
void UBmrCellUtilsLibrary::BreakCell(const FBmrCell& InCell, double& X, double& Y, double& Z)
{
	X = InCell.X();
	Y = InCell.Y();
	Z = InCell.Z();
}

/*********************************************************************************************
 * Transform (Location, Rotation, Scale) on the level
 ********************************************************************************************* */

// Returns transform of cells grid on current level
FTransform UBmrCellUtilsLibrary::GetLevelGridTransform()
{
	// As alternative, GeneratedMap's transform could be taken, but array transform is more reliable
	return GetCellArrayTransform(GetAllCellsOnLevel());
}

// Returns location of grid pivot on current level
FVector UBmrCellUtilsLibrary::GetLevelGridLocation()
{
	// As alternative #1, GeneratedMap's location could be taken, but cell array location is more reliable
	// As alternative #2, GetCellArrayCenter could be used, but it's more expensive
	return GetCenterCellOnLevel().Location;
}

// Returns any cell Z-location on the Generated Map
float UBmrCellUtilsLibrary::GetCellHeightLocation()
{
	return GetLevelGridLocation().Z;
}

// Returns cell rotator that is the same for any cell on the Generated Map
FRotator UBmrCellUtilsLibrary::GetLevelGridRotation()
{
	// As alternative, GeneratedMap's rotation could be taken, but cell array rotation is more reliable
	return GetCellArrayRotation(GetAllCellsOnLevel());
}

// Returns cell yaw angle in degrees that is the same for any cell on the Generated Map
float UBmrCellUtilsLibrary::GetCellYawDegree()
{
	return GetLevelGridRotation().Yaw;
}

// Returns the current grid size
FIntPoint UBmrCellUtilsLibrary::GetLevelGridScale()
{
	// As alternative, GeneratedMap's scale could be taken, but cell array size is more reliable
	const FVector2D GridSize = GetCellArraySize(GetAllCellsOnLevel());
	return FIntPoint(GridSize.X, GridSize.Y);
}

// Returns the width (number of columns X) of the Generated Map
int32 UBmrCellUtilsLibrary::GetCellColumnsNumOnLevel()
{
	return FMath::FloorToInt32(GetCellArrayWidth(GetAllCellsOnLevel()));
}

// Returns the length (number of rows Y) of the Generated Map
int32 UBmrCellUtilsLibrary::GetCellRowsNumOnLevel()
{
	return FMath::FloorToInt32(GetCellArrayLength(GetAllCellsOnLevel()));
}

// Returns GetCellColumnsNumOnLevel - 1
int32 UBmrCellUtilsLibrary::GetLastColumnIndexOnLevel()
{
	return GetCellColumnsNumOnLevel() - 1;
}

// Returns GetCellRowsNumOnLevel - 1
int32 UBmrCellUtilsLibrary::GetLastRowIndexOnLevel()
{
	return GetCellRowsNumOnLevel() - 1;
}

/*********************************************************************************************
 * Generated Map related cell functions
 ********************************************************************************************* */

// Takes the cell and returns its row and column position on the level if exists, -1 otherwise
void UBmrCellUtilsLibrary::GetPositionByCellOnLevel(const FBmrCell& InCell, int32& OutColumnX, int32& OutRowY)
{
	const FIntPoint CellPosition = GetPositionByCellOnGrid(InCell, GetAllCellsOnLevel());
	OutColumnX = CellPosition.X;
	OutRowY = CellPosition.Y;
}

// Returns array index for cell in grid data structure, -1 if not found
int32 UBmrCellUtilsLibrary::GetIndexByCellOnLevel(const FBmrCell& InCell)
{
	if (!InCell.IsValid())
	{
		return INDEX_NONE;
	}

	const FBmrCellsArr& AllGridCells = GetAllCellsOnLevelAsArray();
	return AllGridCells.IndexOfByPredicate([&InCell](const FBmrCell& CellIt)
	{
		return CellIt == InCell;
	});
}

// Gets cell from array index in grid data structure
FBmrCell UBmrCellUtilsLibrary::GetCellByIndexOnLevel(int32 CellIndex)
{
	if (CellIndex < 0)
	{
		return FBmrCell::InvalidCell;
	}

	const FBmrCellsArr& AllGridCells = GetAllCellsOnLevelAsArray();
	return AllGridCells.IsValidIndex(CellIndex) ? AllGridCells[CellIndex] : FBmrCell::InvalidCell;
}

// Returns all grid cell location on the Generated Map
const FBmrCellsArr& UBmrCellUtilsLibrary::GetAllCellsOnLevelAsArray()
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	return GeneratedMap ? GeneratedMap->LocalGridCells : FBmrCell::EmptyCellsArr;
}

// Returns the cell location of the Generated Map
FBmrCell UBmrCellUtilsLibrary::GetCenterCellOnLevel()
{
	int32 CenterColumn = INDEX_NONE;
	int32 CenterRow = INDEX_NONE;
	GetCenterCellPositionOnLevel(/*out*/ CenterRow, /*out*/ CenterColumn);
	return GetCellByPositionOnLevel(CenterColumn, CenterRow);
}

// Returns the center row and column positions on the level
void UBmrCellUtilsLibrary::GetCenterCellPositionOnLevel(int32& OutColumnX, int32& OutRowY)
{
	const FIntPoint CenterCellPosition = GetCenterCellPositionOnGrid(GetAllCellsOnLevel());
	OutColumnX = CenterCellPosition.X;
	OutRowY = CenterCellPosition.Y;
}

// Returns the center cell location on the level
FIntPoint UBmrCellUtilsLibrary::GetCenterCellPositionOnLevel()
{
	int32 OutColumnX;
	int32 OutRowY;
	GetCenterCellPositionOnLevel(/*out*/ OutColumnX, /*out*/ OutRowY);
	return FIntPoint(OutColumnX, OutRowY);
}

// Returns all empty grid cell locations on the Generated Map where none of actors are present
FBmrCells UBmrCellUtilsLibrary::GetAllEmptyCellsWithoutActors()
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return GetAllCellsWithActors(NoneActorType);
}

// Returns all grid cell location on the Generated Map by specified actor types
FBmrCells UBmrCellUtilsLibrary::GetAllCellsWithActors(int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = true;
	FBmrCells OutCells = FBmrCell::EmptyCells;
	IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// The intersection of (OutCells ∩ ActorsTypesBitmask).
void UBmrCellUtilsLibrary::IntersectCellsByTypes(FBmrCells& InOutCells, int32 ActorsTypesBitmask, bool bIntersectAllIfEmpty)
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	if (!GeneratedMap) // Might be null if called before the map is initialized
	{
		return;
	}

	const FBmrCellsArr& AllGridCells = GetAllCellsOnLevelAsArray();

	if (AllGridCells.IsEmpty())
	{
		// nothing to intersect
		return;
	}

	if (!bIntersectAllIfEmpty && !InOutCells.Num())
	{
		// should not intersect with all existed cells but the specified array is empty
		return;
	}

	if (!ActorsTypesBitmask)
	{
		// Find all empty grid cell locations where none of actors are present
		const FBmrCells AllEmptyCells = FBmrCells(
		    AllGridCells.FilterByPredicate([&MapComponents = GeneratedMap->MapComponents](const FBmrCell& CellIt)
		{
			return !MapComponents.Contains(CellIt);
		}));

		if (InOutCells.Num())
		{
			InOutCells = InOutCells.Intersect(AllEmptyCells);
		}
		else
		{
			InOutCells = AllEmptyCells;
		}

		return;
	}

	FMapComponents BitmaskedComponents;
	UBmrActorUtilsLibrary::GetLevelActors(BitmaskedComponents, ActorsTypesBitmask);
	if (!BitmaskedComponents.Num())
	{
		InOutCells.Empty(); // nothing found, returns empty OutCells array
		return;
	}

	FBmrCells BitmaskedCells;
	for (const UBmrMapComponent* MapCompIt : BitmaskedComponents)
	{
		if (MapCompIt)
		{
			BitmaskedCells.Emplace(MapCompIt->GetCell());
		}
	}

	if (InOutCells.Num())
	{
		InOutCells = InOutCells.Intersect(BitmaskedCells);
	}
	else
	{
		InOutCells = BitmaskedCells;
	}
}

// Takes cells and returns only empty cells where none of actors are present
FBmrCells UBmrCellUtilsLibrary::FilterEmptyCellsWithoutActors(const FBmrCells& InCells)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return FilterCellsByActors(InCells, NoneActorType);
}

// Takes cells and returns only matching with specified actor types
FBmrCells UBmrCellUtilsLibrary::FilterCellsByActors(const FBmrCells& InCells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FBmrCells OutCells = InCells;
	IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// Returns true if cell is empty, so it does not have own actor
bool UBmrCellUtilsLibrary::IsEmptyCellWithoutActor(const FBmrCell& Cell)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return IsCellHasAnyMatchingActor(Cell, NoneActorType);
}

// Checking the containing of the specified cell among owners locations of the Map Components array
bool UBmrCellUtilsLibrary::IsCellHasAnyMatchingActor(const FBmrCell& Cell, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FBmrCells NonEmptyCells{Cell};
	IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Contains(Cell);
}

// Returns true if at least one cell is empty, so it does not have own actor
bool UBmrCellUtilsLibrary::IsAnyCellEmptyWithoutActor(const FBmrCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return AreCellsHaveAnyMatchingActors(Cells, NoneActorType);
}

// Returns true if at least one cell has actors of specified types
bool UBmrCellUtilsLibrary::AreCellsHaveAnyMatchingActors(const FBmrCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FBmrCells NonEmptyCells{Cells};
	IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return !NonEmptyCells.IsEmpty();
}

// Returns true if all cells are empty, so don't have own actors
bool UBmrCellUtilsLibrary::AreAllCellsEmptyWithoutActors(const FBmrCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return AreCellsHaveAllMatchingActors(Cells, NoneActorType);
}

// Returns true if all cells have actors of specified types
bool UBmrCellUtilsLibrary::AreCellsHaveAllMatchingActors(const FBmrCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FBmrCells NonEmptyCells{Cells};
	IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Num() == Cells.Num();
}

// Getting an array of cells by four sides of an input center cell and type of breaks
void UBmrCellUtilsLibrary::GetSideCells(FBmrCells& OutCells, const FBmrCell& Cell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask, bool bBreakInputCells)
{
	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	const int32 MaxWidth = GetCellColumnsNumOnLevel();
	if (!GeneratedMap // Might be null if called before the map is initialized
	    || !ensureMsgf(MaxWidth, TEXT("ASSERT: Level has zero width (Scale.X)"))
	    || !ensureMsgf(DirectionsBitmask, TEXT("ASSERT: 'DirectionsBitmask' is not set"))
	    || !ensureMsgf(SideLength > 0, TEXT("ASSERT: 'SideLength' is less than 1"))
	    || !ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is invalid")))
	{
		return;
	}

	const FBmrCellsArr& AllGridCells = GetAllCellsOnLevelAsArray();
	const FBmrCells& AdditionalDangerousCells = GeneratedMap->AdditionalDangerousCells;

	const bool bIsAnyPath = Pathfinder == EPathType::Any;

	// ----- Walls definition -----
	FBmrCells Walls;
	bool bBreakOnWalls = !bIsAnyPath && !OutCells.Num();
	if (bBreakOnWalls)
	{
		Walls = GetAllCellsWithActors(TO_FLAG(EAT::Wall));
	}
	else if (bBreakInputCells) // specified OutCells is not empty, these cells break lines as the Wall behavior
	{
		bBreakOnWalls = true;
		Walls = OutCells; // these cells break lines as the Wall behavior, don't empty specified array
	}
	else // !bBreakOnWalls && !bBreakInputCells
	{
		OutCells.Empty(); // should empty array in order to return only sides cells
	}

	// the index of the specified cell
	const int32 C0 = AllGridCells.IndexOfByPredicate([&Cell](const FBmrCell& InCell)
	{
		return InCell == Cell;
	});
	if (C0 == INDEX_NONE) // if index was found and cell is contained in the array
	{
		return;
	}

	// ----- A path without obstacles -----
	FBmrCells Obstacles;
	const bool bBreakOnObstacles = !bIsAnyPath && Pathfinder != EPathType::Explosion;
	if (bBreakOnObstacles) // if is the request to find the path without Bombs/Boxes
	{
		Obstacles = GetAllCellsWithActors(TO_FLAG(EAT::Bomb | EAT::Box));
	}

	// ----- Secure: a path without players -----
	FBmrCells PlayersCells;
	const bool bBreakOnPlayers = Pathfinder == EPathType::Secure;
	if (bBreakOnPlayers) // if is the request to find the path without players cells.
	{
		PlayersCells = GetAllCellsWithActors(TO_FLAG(EAT::Player));
	}

	// ----- A path without explosions -----
	FBmrCells DangerousCells = AdditionalDangerousCells;
	const bool bBreakOnExplosions = Pathfinder == EPathType::Safe || Pathfinder == EPathType::Secure;
	if (bBreakOnExplosions) // if is the request to find the path without explosions.
	{
		DangerousCells.Append(GetAllExplosionCells());
	}

	// ----- The specified cell adding -----
	if (!bBreakOnExplosions // can be dangerous cell
	    || !DangerousCells.Contains(Cell)) // is not dangerous cell
	{
		OutCells.Emplace(Cell);
	}

	// ----- Cells finding -----
	for (int8 bIsY = 0; bIsY <= 1; ++bIsY) // 0(X-raw direction) and 1(Y-column direction)
	{
		const int32 PositionC0 = bIsY ? /*Y-column*/ C0 % MaxWidth : C0 / MaxWidth /*raw*/;
		for (int8 SideMultiplier = -1; SideMultiplier <= 1; SideMultiplier += 2) // -1(Left|Down) and 1(Right|Up)
		{
			const FVector& VectorDirection = bIsY ? FVector::BackwardVector : FVector::RightVector;
			const FBmrCell CellDirection = VectorDirection * FVector(SideMultiplier);
			const EBmrCellDirection EnumDirection = FBmrCell::GetCellDirection(CellDirection);
			if (!EnumHasAnyFlags(EnumDirection, TO_ENUM(EBmrCellDirection, DirectionsBitmask)))
			{
				continue;
			}

			for (int8 i = 1; i <= SideLength; ++i)
			{
				int32 Distance = i * SideMultiplier;
				if (bIsY)
				{
					Distance *= MaxWidth;
				}
				const int32 FoundIndex = C0 + Distance;
				if (PositionC0 != (bIsY ? FoundIndex % MaxWidth : FoundIndex / MaxWidth) // PositionC0 != PositionX
				    || !AllGridCells.IsValidIndex(FoundIndex)) // is not in range
				{
					break; // to the next side
				}

				const FBmrCell FoundCell = AllGridCells[FoundIndex];

				if (bBreakOnWalls
				    && Walls.Contains(FoundCell))
				{
					// cell contains a wall
					break;
				}

				if (bBreakOnObstacles
				    && Obstacles.Contains(FoundCell))
				{
					// cell contains an obstacle (Bombs/Boxes)
					break;
				}

				if (bBreakOnPlayers
				    && PlayersCells.Contains(FoundCell))
				{
					// cell contains a player
					break;
				}

				if (bBreakOnExplosions
				    && DangerousCells.Contains(FoundCell))
				{
					// cell contains an explosion
					break;
				}

				OutCells.Emplace(FoundCell);
			} // Cells iterating
		} // Each side iterating: -1(Left|Down) and 1(Right|Up)
	} // Each direction iterating: 0(X-raw) and 1(Y-column)
}

FBmrCells UBmrCellUtilsLibrary::GetCellsAround(const FBmrCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 AllDirections = TO_FLAG(EBmrCellDirection::All);
	FBmrCells OutCells = FBmrCell::EmptyCells;
	GetSideCells(OutCells, CenterCell, Pathfinder, Radius, AllDirections);
	return OutCells;
}

// Returns cells that match specified actors in specified radius from a center, according desired type of breaks
FBmrCells UBmrCellUtilsLibrary::GetCellsAroundWithActors(const FBmrCell& CenterCell, EPathType Pathfinder, int32 Radius, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FBmrCells CellsAround = GetCellsAround(CenterCell, Pathfinder, Radius);
	return FilterCellsByActors(CellsAround, ActorsTypesBitmask);
}

// Returns matching empty cells around without actors, according desired type of breaks
FBmrCells UBmrCellUtilsLibrary::GetEmptyCellsAroundWithoutActors(const FBmrCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return GetCellsAroundWithActors(CenterCell, Pathfinder, Radius, NoneActorType);
}

// Returns first cell in specified direction from a center
FBmrCell UBmrCellUtilsLibrary::GetCellInDirection(const FBmrCell& CenterCell, EPathType Pathfinder, EBmrCellDirection Direction)
{
	FBmrCells OutCells = FBmrCell::EmptyCells;
	ensureMsgf(Direction != EBmrCellDirection::All, TEXT("ASSERT: Is specified 'EBmrCellDirection::All' while function could return only cell in 1 direction"));
	constexpr int32 SideLength = 1;
	GetSideCells(OutCells, CenterCell, Pathfinder, SideLength, TO_FLAG(Direction));
	OutCells.Remove(CenterCell);
	return FBmrCell::GetFirstCellInSet(OutCells);
}

// Returns true if a cell was found in specified direction from a center, according desired type of breaks
bool UBmrCellUtilsLibrary::CanGetCellInDirection(const FBmrCell& CenterCell, EPathType Pathfinder, EBmrCellDirection Direction)
{
	return GetCellInDirection(CenterCell, Pathfinder, Direction).IsValid();
}

// Returns cells in specified direction from a center
FBmrCells UBmrCellUtilsLibrary::GetCellsInDirections(const FBmrCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	FBmrCells OutCells = FBmrCell::EmptyCells;
	GetSideCells(OutCells, CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return OutCells;
}

// Returns cells that match specified actors in specified direction from a center
FBmrCells UBmrCellUtilsLibrary::GetCellsInDirectionsWithActors(const FBmrCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FBmrCells CellsInDirections = GetCellsInDirections(CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return FilterCellsByActors(CellsInDirections, ActorsTypesBitmask);
}

// Returns matching empty cells without actors in chosen direction(s), according desired type of breaks
FBmrCells UBmrCellUtilsLibrary::GetEmptyCellsInDirectionsWithoutActors(const FBmrCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	constexpr int32 NoneActorType = TO_FLAG(EBmrLevelType::None);
	return GetCellsInDirectionsWithActors(CenterCell, Pathfinder, SideLength, DirectionsBitmask, NoneActorType);
}

// Returns true if player is not able to reach specified cell by any path
bool UBmrCellUtilsLibrary::IsIslandCell(const FBmrCell& Cell)
{
	return !DoesPathExistToCellsOnLevel({Cell}, FBmrCell::EmptyCells);
}

// Rotates the given cell around the center of the Generated Map to the same yaw degree
FBmrCell UBmrCellUtilsLibrary::RotateCellAroundLevelOrigin(const FBmrCell& Cell, float AxisZ)
{
	const FTransform GridTransformNoScale = FBmrCell::GetCellArrayTransformNoScale(GetAllCellsOnLevel());
	return FBmrCell::RotateCellAroundOrigin(Cell, AxisZ, GridTransformNoScale);
}

// Gets actor location snapped to nearest cell on the level grid
FBmrCell UBmrCellUtilsLibrary::SnapActorOnLevel(const AActor* Actor)
{
	return Actor ? SnapVectorOnLevel(Actor->GetActorLocation()) : FBmrCell::InvalidCell;
}

// Returns nearest free cell to given cell, where free means cell with no other level actors except players
FBmrCell UBmrCellUtilsLibrary::GetNearestFreeCell(const FBmrCell& Cell)
{
	FBmrCells FreeCells = GetAllEmptyCellsWithoutActors();
	FreeCells.Append(GetAllCellsWithActors(TO_FLAG(EAT::Player))); // Players are also considered as free cells
	return GetCellArrayNearest(FreeCells, Cell);
}

// Returns all explosion cells on Level
TSet<FBmrCell> UBmrCellUtilsLibrary::GetAllExplosionCells()
{
	FMapComponents BombMapComponents;
	UBmrActorUtilsLibrary::GetLevelActors(BombMapComponents, TO_FLAG(EAT::Bomb));

	FBmrCells ExplosionCells = FBmrCell::EmptyCells;
	for (const UBmrMapComponent* MapComponentIt : BombMapComponents)
	{
		const ABmrBombAbilityActor* BombActor = MapComponentIt ? MapComponentIt->GetOwner<ABmrBombAbilityActor>() : nullptr;
		if (BombActor)
		{
			ExplosionCells.Append(BombActor->GetExplosionCells());
		}
	}

	return ExplosionCells;
}

// Returns true if any player is able to reach all specified cells by any path
bool UBmrCellUtilsLibrary::DoesPathExistToCellsOnLevel(const TSet<FBmrCell>& CellsToFind, const TSet<FBmrCell>& OptionalPathBreakers)
{
	// If there are no targets to find, the condition is trivially met.
	if (CellsToFind.IsEmpty())
	{
		return true;
	}

	const FBmrCellsArr& AllGridCells = GetAllCellsOnLevelAsArray();
	if (AllGridCells.IsEmpty())
	{
		// Cannot find a path on a non-existent level.
		return false;
	}

	TQueue<FBmrCell> CellsToVisitQueue;
	TSet<FBmrCell> VisitedCells = OptionalPathBreakers;
	TSet<FBmrCell> FoundTargets;

	constexpr int32 RootCell = 0;
	const FBmrCell& StartCell = AllGridCells[RootCell];

	CellsToVisitQueue.Enqueue(StartCell);
	VisitedCells.Add(StartCell);

	// Check if the starting cell itself is one of the targets.
	if (CellsToFind.Contains(StartCell))
	{
		FoundTargets.Add(StartCell);

		// Handle the edge case where the origin is the only target.
		if (FoundTargets.Num() == CellsToFind.Num())
		{
			return true;
		}
	}

	// --- BFS Main Loop ---
	while (!CellsToVisitQueue.IsEmpty())
	{
		FBmrCell CurrentCell;
		CellsToVisitQueue.Dequeue(CurrentCell);

		static constexpr int32 SearchRadius = 1;
		const FBmrCells Neighbors = GetCellsAround(CurrentCell, EPathType::Explosion, SearchRadius);

		for (const FBmrCell& Neighbor : Neighbors)
		{
			if (VisitedCells.Contains(Neighbor))
			{
				continue;
			}

			VisitedCells.Add(Neighbor);
			CellsToVisitQueue.Enqueue(Neighbor);

			if (CellsToFind.Contains(Neighbor))
			{
				FoundTargets.Add(Neighbor);

				if (FoundTargets.Num() == CellsToFind.Num())
				{
					return true;
				}
			}
		}
	}

	return false;
}

// Return closest corner cell to the given cell
FBmrCell UBmrCellUtilsLibrary::GetNearestCornerCellOnLevel(const FBmrCell& CellToCheck)
{
	const TSet<FBmrCell> AllCornerCells = GetCornerCellsOnLevel();
	return GetCellArrayNearest(AllCornerCells, CellToCheck);
}

/*********************************************************************************************
 * Debug cells utilities
 ********************************************************************************************* */

// Remove all text renders of the Owner, is not available in shipping build
void UBmrCellUtilsLibrary::ClearDisplayedCells(const UObject* Owner /* = nullptr*/)
{
#if !UE_BUILD_SHIPPING
	const AActor* OwnerActor = Cast<AActor>(Owner);
	if (!OwnerActor)
	{
		const UActorComponent* Component = Cast<UActorComponent>(Owner);
		OwnerActor = Component ? Component->GetOwner() : nullptr;
		if (!OwnerActor)
		{
			// Owner is not provided, fallback to the default Generated Map
			OwnerActor = ABmrGeneratedMap::GetGeneratedMap();
		}
	}

	TArray<UTextRenderComponent*> TextRendersArray;
	OwnerActor->GetComponents<UTextRenderComponent>(TextRendersArray);
	for (int32 Index = TextRendersArray.Num() - 1; Index >= 0; --Index)
	{
		UTextRenderComponent* TextRenderIt = TextRendersArray.IsValidIndex(Index) ? Cast<UTextRenderComponent>(TextRendersArray[Index]) : nullptr;
		if (IsValid(TextRenderIt) // is not pending kill
		    && TextRenderIt->HasAllFlags(RF_Transient)) // cell text renders have this flag
		{
			TextRenderIt->DestroyComponent();
		}
	}
#endif // !UE_BUILD_SHIPPING
}

// Display coordinates of specified cells on the level, is not available in shipping build
void UBmrCellUtilsLibrary::DisplayCells(UObject* Owner, const FBmrCells& Cells, const FBmrDisplayCellsParams& Params)
{
#if !UE_BUILD_SHIPPING
	if (Cells.IsEmpty())
	{
		return;
	}

	AActor* OwnerActor = Cast<AActor>(Owner);
	if (!OwnerActor)
	{
		const UActorComponent* Component = Cast<UActorComponent>(Owner);
		OwnerActor = Component ? Component->GetOwner() : nullptr;
		if (!OwnerActor)
		{
			// Owner is not provided, fallback to the default Generated Map
			OwnerActor = ABmrGeneratedMap::GetGeneratedMap();
		}
	}

	// Firstly, try to clear previous displays first
	if (Params.bClearPreviousDisplays)
	{
		ClearDisplayedCells(OwnerActor);
	}

	// Next, try to display new cells (if allowed)
	if (!CanDisplayCells(OwnerActor))
	{
		return;
	}

	// Have the render text rotated
	const FQuat CellGridQuaternion = GetLevelGridRotation().Quaternion();

	for (const FBmrCell& CellIt : Cells)
	{
		if (CellIt.IsInvalidCell())
		{
			continue;
		}

		const bool bHasAdditionalRenderString = !Params.RenderString.IsNone();
		const bool bHasCoordinatePosition = !Params.CoordinatePosition.IsZero();
		constexpr int32 MaxCoordinateRenders = 2;
		for (int32 Index = 0; Index < MaxCoordinateRenders; ++Index)
		{
			enum ERenderType
			{
				ERT_Coordinate,
				ERT_RenderString
			};
			const ERenderType RenderType = Index == 0 ? ERT_Coordinate : ERT_RenderString;
			const bool bShowRenderString = RenderType == ERT_RenderString && bHasAdditionalRenderString;
			const bool bShowCoordinate = RenderType == ERT_Coordinate && (bHasCoordinatePosition || !bHasAdditionalRenderString);

			if (!bShowCoordinate && !bShowRenderString)
			{
				continue;
			}

			UTextRenderComponent& RenderComp = *NewObject<UTextRenderComponent>(OwnerActor, NAME_None, RF_Transient);
			RenderComp.RegisterComponent();

			// Get text render location on each cell
			FVector TextLocation = FVector::ZeroVector;
			const FVector CellLocation(CellIt.X(), CellIt.Y(), GetCellHeightLocation() + Params.TextHeight);
			if (bShowCoordinate)
			{
				TextLocation.X = CellLocation.X + Params.CoordinatePosition.X * -1.f;
				TextLocation.Y = CellLocation.Y + Params.CoordinatePosition.Y * -1.f;
				TextLocation.Z = CellLocation.Z + Params.CoordinatePosition.Z;
			}
			else if (bShowRenderString)
			{
				TextLocation = CellLocation + Params.CoordinatePosition;
			}

			// --- Init the text render

			if (bShowCoordinate)
			{
				const FString XString = FString::FromInt(FMath::FloorToInt32(CellLocation.X));
				const FString YString = FString::FromInt(FMath::FloorToInt32(CellLocation.Y));
				constexpr float DelimiterTextSize = 48.f;
				const FString DelimiterString = Params.TextSize > DelimiterTextSize ? TEXT("\n") : TEXT("");
				const FString RenderString = XString + DelimiterString + YString;
				RenderComp.SetText(FText::FromString(RenderString));
			}
			else if (bShowRenderString)
			{
				// Display RenderString as is
				FText NameToStringToText = FText::FromString(Params.RenderString.ToString());
				RenderComp.SetText(NameToStringToText);
			}

			RenderComp.SetAbsolute(true, true, true);

			if (bShowCoordinate)
			{
				constexpr float CoordinateSizeMultiplier = 0.4f;
				RenderComp.SetWorldSize(Params.TextSize * CoordinateSizeMultiplier);
			}
			else if (bShowRenderString)
			{
				RenderComp.SetWorldSize(Params.TextSize);
			}

			RenderComp.SetHorizontalAlignment(EHTA_Center);
			RenderComp.SetVerticalAlignment(EVRTA_TextCenter);

			RenderComp.SetTextRenderColor(Params.TextColor.ToFColor(true));

			FTransform RenderTransform = FTransform::Identity;
			RenderTransform.SetLocation(TextLocation);
			static const FQuat AdditiveQuat = FRotator(90.f, 0.f, -90.f).Quaternion();
			RenderTransform.SetRotation(CellGridQuaternion * AdditiveQuat);
			RenderComp.SetWorldTransform(RenderTransform);
		}
	}
#endif // !UE_BUILD_SHIPPING
}

// Returns true if cells of specified actor type(s) can be displayed
bool UBmrCellUtilsLibrary::CanDisplayCells(const UObject* Owner)
{
#if !UE_BUILD_SHIPPING
	if (!Owner)
	{
		return false;
	}

	const AActor* OwnerActor = Cast<AActor>(Owner);
	if (!OwnerActor)
	{
		const UActorComponent* Component = Cast<UActorComponent>(Owner);
		OwnerActor = Component ? Component->GetOwner() : nullptr;
		if (!OwnerActor)
		{
			// Can't display cells for this owner
			return false;
		}
	}

	const UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(OwnerActor);
	if (!MapComponent)
	{
		// It's custom actor without Map Component, always allow by default
		return true;
	}

	const int32 ActorTypesBitmask = TO_FLAG(MapComponent->GetActorType());
	const FString CheatValue = UBmrCheatManager::CVarDisplayCells.GetValueOnAnyThread();
	const bool bAllowedFromCheat = (ActorTypesBitmask & UBmrCheatManager::GetBitmaskFromActorTypesString(CheatValue)) != 0;
	if (bAllowedFromCheat)
	{
		return true;
	}

	const ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap();
	const bool bAllowedGlobally = GeneratedMap && (ActorTypesBitmask & ABmrGeneratedMap::Get().DisplayCellsActorTypes) != 0;
	if (bAllowedGlobally)
	{
		return true;
	}

	const bool bAllowedLocally = MapComponent->bShouldShowRenders;
	return bAllowedLocally;
#else
	return false;
#endif
}