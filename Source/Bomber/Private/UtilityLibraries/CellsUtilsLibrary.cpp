// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "GeneratedMap.h"

// Returns the cell by specified row and column number if exists, invalid cell otherwise
const FCell& UCellsUtilsLibrary::GetCellOnLevel(int32 Row, int32 Column)
{
	const int32 MaxWidth = GetCellColumnsNumOnLevel();
	const int32 CellIndex = Row * MaxWidth + Column;
	const TArray<FCell>& AllCells = GetAllCellsOnLevelAsArray();
	return AllCells.IsValidIndex(CellIndex) ? AllCells[CellIndex] : FCell::InvalidCell;
}

// Takes the cell and returns its row and column position on the level if exists, -1 otherwise
void UCellsUtilsLibrary::GetCellPositionOnLevel(const FCell& InCell, int32& OutRow, int32& OutColumn)
{
	const int32 MaxWidth = AGeneratedMap::Get().GetCachedTransform().GetScale3D().X;
	const int32 CellIdx = GetAllCellsOnLevelAsArray().IndexOfByPredicate([&InCell](const FCell& CellIt) { return CellIt == InCell; });
	const bool bFound = CellIdx != INDEX_NONE && MaxWidth;
	OutRow = bFound ? CellIdx / MaxWidth : INDEX_NONE;
	OutColumn = bFound ? CellIdx % MaxWidth : INDEX_NONE;
}

// Returns all grid cell location on the Level Map
FCells UCellsUtilsLibrary::GetAllCellsOnLevel()
{
	return FCells{AGeneratedMap::Get().GridCellsInternal};
}

// Returns all grid cell location on the Level Map
const TArray<FCell>& UCellsUtilsLibrary::GetAllCellsOnLevelAsArray()
{
	return AGeneratedMap::Get().GridCellsInternal;
}

// Returns the cell location of the Level Map
FCell UCellsUtilsLibrary::GetCenterCellOnLevel()
{
	return FCell(AGeneratedMap::Get().GetCachedTransform().GetLocation());
}

// Returns the number of columns on the Level Map
int32 UCellsUtilsLibrary::GetCellColumnsNumOnLevel()
{
	return AGeneratedMap::Get().GetCachedTransform().GetScale3D().X;
}

// Returns the number of rows on the Level Map
int32 UCellsUtilsLibrary::GetCellRowsNumOnLevel()
{
	return AGeneratedMap::Get().GetCachedTransform().GetScale3D().Y;
}

// Returns GetCellColumnsNumOnLevel - 1
int32 UCellsUtilsLibrary::GetLastColumnIndexOnLevel()
{
	return GetCellColumnsNumOnLevel() - 1;
}

// Returns Returns GetCellRowsNumOnLevel - 1
int32 UCellsUtilsLibrary::GetLastRowIndexOnLevel()
{
	return GetCellRowsNumOnLevel() - 1;
}

// Returns any cell rotation on the Level Map
float UCellsUtilsLibrary::GetCellRotation()
{
	return AGeneratedMap::Get().GetCachedTransform().GetRotation().Rotator().Yaw;
}

// Returns any cell Z-location on the Level Map
float UCellsUtilsLibrary::GetCellHeightLocation()
{
	return AGeneratedMap::Get().GetCachedTransform().GetLocation().Z;
}

// Returns all empty grid cell locations on the Level Map where non of actors are present
FCells UCellsUtilsLibrary::GetAllEmptyCellsWithoutActors()
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetAllCellsWithActors(NoneActorType);
}

// Returns all grid cell location on the Level Map by specified actor types
FCells UCellsUtilsLibrary::GetAllCellsWithActors(int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = true;
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// Takes cells and returns only empty cells where non of actors are present
FCells UCellsUtilsLibrary::FilterEmptyCellsWithoutActors(const FCells& InCells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return FilterCellsByActors(InCells, NoneActorType);
}

// Takes cells and returns only matching with specified actor types
FCells UCellsUtilsLibrary::FilterCellsByActors(const FCells& InCells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells OutCells = InCells;
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return OutCells;
}

// Returns true if cell is empty, so it does not have own actor
bool UCellsUtilsLibrary::IsEmptyCellWithoutActor(const FCell& Cell)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return IsCellHasAnyMatchingActor(Cell, NoneActorType);
}

// Checking the containing of the specified cell among owners locations of the Map Components array
bool UCellsUtilsLibrary::IsCellHasAnyMatchingActor(const FCell& Cell, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cell};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Contains(Cell);
}

// Returns true if at least one cell is empty, so it does not have own actor
bool UCellsUtilsLibrary::IsAnyCellEmptyWithoutActor(const FCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return AreCellsHaveAnyMatchingActors(Cells, NoneActorType);
}

// Returns true if at least one cell has actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAnyMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return !NonEmptyCells.IsEmpty();
}

// Returns true if all cells are empty, so don't have own actors
bool UCellsUtilsLibrary::AreAllCellsEmptyWithoutActors(const FCells& Cells)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return AreCellsHaveAllMatchingActors(Cells, NoneActorType);
}

// Returns true if all cells have actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAllMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Num() == Cells.Num();
}

// Returns true if specified cell is present on the Level Map.
bool UCellsUtilsLibrary::IsCellExistsOnLevel(const FCell& Cell)
{
	return Cell.IsValid() && AGeneratedMap::Get().GridCellsInternal.Contains(Cell);
}

// Returns true if the cell is present on the Level Map with such row and column indexes
bool UCellsUtilsLibrary::IsCellPositionExistsOnLevel(int32 Row, int32 Column)
{
	return GetCellOnLevel(Row, Column).IsValid();
}

// Returns true if at least one cell is present on the Level Map
bool UCellsUtilsLibrary::IsAnyCellExistsOnLevel(const FCells& Cells)
{
	return FCells{AGeneratedMap::Get().GridCellsInternal}.Intersect(Cells).Num() > 0;
}

// Returns true if all specified cells are present on the Level Map
bool UCellsUtilsLibrary::AreAllCellsExistOnLevel(const FCells& Cells)
{
	return FCells{AGeneratedMap::Get().GridCellsInternal}.Includes(Cells);
}

// Returns cells around the center in specified radius and according desired type of breaks
FCells UCellsUtilsLibrary::GetCellsAround(const FCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 AllDirections = TO_FLAG(ECellDirection::All);
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, Radius, AllDirections);
	return OutCells;
}

// Returns cells that match specified actors in specified radius from a center, according desired type of breaks
FCells UCellsUtilsLibrary::GetCellsAroundWithActors(const FCell& CenterCell, EPathType Pathfinder, int32 Radius, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FCells CellsAround = GetCellsAround(CenterCell, Pathfinder, Radius);
	return FilterCellsByActors(CellsAround, ActorsTypesBitmask);
}

// Returns matching empty cells around without actors, according desired type of breaks
FCells UCellsUtilsLibrary::GetEmptyCellsAroundWithoutActors(const FCell& CenterCell, EPathType Pathfinder, int32 Radius)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetCellsAroundWithActors(CenterCell, Pathfinder, Radius, NoneActorType);
}

// Returns first cell in specified direction from a center
FCell UCellsUtilsLibrary::GetCellInDirection(const FCell& CenterCell, EPathType Pathfinder, ECellDirection Direction)
{
	ensureMsgf(Direction != ECellDirection::All, TEXT("ASSERT: Is specified 'ECellDirection::All' while function could return only cell in 1 direction"));
	constexpr int32 SideLength = 1;
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, SideLength, TO_FLAG(Direction));
	OutCells.Remove(CenterCell);
	return FCell::GetFirstCellInSet(OutCells);
}

// Returns true if a cell was found in specified direction from a center, according desired type of breaks
bool UCellsUtilsLibrary::CanGetCellInDirection(const FCell& CenterCell, EPathType Pathfinder, ECellDirection Direction)
{
	return GetCellInDirection(CenterCell, Pathfinder, Direction).IsValid();
}

// Returns cells in specified direction from a center
FCells UCellsUtilsLibrary::GetCellsInDirections(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	FCells OutCells = FCell::EmptyCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return OutCells;
}

// Returns cells that match specified actors in specified direction from a center
FCells UCellsUtilsLibrary::GetCellsInDirectionsWithActors(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask, int32 ActorsTypesBitmask)
{
	ensureMsgf(Pathfinder == EPathType::Any || !EnumHasAnyFlags(EAT::Wall, TO_ENUM(EAT, ActorsTypesBitmask)), TEXT("ASSERT: Is trying to find walls for a pathfinder that breaks a path by walls"));
	const FCells CellsInDirections = GetCellsInDirections(CenterCell, Pathfinder, SideLength, DirectionsBitmask);
	return FilterCellsByActors(CellsInDirections, ActorsTypesBitmask);
}

// Returns matching empty cells without actors in chosen direction(s), according desired type of breaks
FCells UCellsUtilsLibrary::GetEmptyCellsInDirectionsWithoutActors(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, int32 DirectionsBitmask)
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetCellsInDirectionsWithActors(CenterCell, Pathfinder, SideLength, DirectionsBitmask, NoneActorType);
}

// Returns true if player is not able to reach specified cell by any any path
bool UCellsUtilsLibrary::IsIslandCell(const FCell& Cell)
{
	return !AGeneratedMap::Get().DoesPathExistToCells({Cell});
}
