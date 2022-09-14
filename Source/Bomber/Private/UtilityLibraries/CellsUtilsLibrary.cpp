// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "GeneratedMap.h"

// Returns all grid cell location on the Level Map
void UCellsUtilsLibrary::GetAllCellsOnLevel(FCells& OutCells)
{
	OutCells = FCells{AGeneratedMap::Get().GridCellsInternal};
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

// Returns all grid cell location on the Level Map by specified actor types
auto UCellsUtilsLibrary::GetAllCellsByActors(FCells& OutCells, int32 ActorsTypesBitmask) -> void
{
	constexpr bool bIntersectAllIfEmpty = true;
	OutCells.Empty();
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
}

// Takes cells and returns only matching with specified actor types
void UCellsUtilsLibrary::FilterCellsByActors(const FCells& InCells, FCells& OutCells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	OutCells = InCells;
	AGeneratedMap::Get().IntersectCellsByTypes(OutCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
}

// Checking the containing of the specified cell among owners locations of the Map Components array
bool UCellsUtilsLibrary::IsCellHasAnyMatchingActor(const FCell& Cell, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cell};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Contains(Cell);
}

// Returns true if at least one cell has actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAnyMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Num() > 0;
}

// Returns true if all cells have actors of specified types
bool UCellsUtilsLibrary::AreCellsHaveAllMatchingActors(const FCells& Cells, int32 ActorsTypesBitmask)
{
	constexpr bool bIntersectAllIfEmpty = false;
	FCells NonEmptyCells{Cells};
	AGeneratedMap::Get().IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask, bIntersectAllIfEmpty);
	return NonEmptyCells.Num() == Cells.Num();
}
