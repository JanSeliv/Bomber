// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "GeneratedMap.h"

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

// Returns all empty grid cell locations on the Level Map where non of actors are present
FCells UCellsUtilsLibrary::GetAllEmptyCellsWithoutActors()
{
	constexpr int32 NoneActorType = TO_FLAG(ELevelType::None);
	return GetAllCellsByActors(NoneActorType);
}

// Returns all grid cell location on the Level Map by specified actor types
FCells UCellsUtilsLibrary::GetAllCellsByActors(int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = true;
	FCells OutCells;
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
	return AGeneratedMap::Get().GridCellsInternal.Contains(Cell);
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
	FCells OutCells;
	AGeneratedMap::Get().GetSidesCells(OutCells, CenterCell, Pathfinder, Radius);
	return OutCells;
}

// Returns first cell in specified direction from a center
FCell UCellsUtilsLibrary::GetCellInDirection(const FCell& CenterCell, EPathType Pathfinder, ECellDirection Direction)
{
	constexpr int32 SideLength = 1;
	const FCells Cells = GetCellsInDirection(CenterCell, Pathfinder, SideLength, Direction);
	return !Cells.IsEmpty() ? Cells.Array()[0] : FCell::ZeroCell;
}

// Returns cells in specified direction from a center
FCells UCellsUtilsLibrary::GetCellsInDirection(const FCell& CenterCell, EPathType Pathfinder, int32 SideLength, ECellDirection Direction)
{
	FCells OutCells;
	const FCell& CellDirection = FCell::GetCellDirection(Direction);
	//@TODO implement next
	//AGeneratedMap::Get().GetSidesCellInDirection(OutCells, CenterCell, Pathfinder, SideLength, CellDirection);
	return OutCells;
}
