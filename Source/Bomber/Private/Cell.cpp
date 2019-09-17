// Copyright 2019 Yevhenii Selivanov.

#include "Cell.h"

#include "GameFramework/Actor.h"

#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"

// The zero cell
const FCell FCell::ZeroCell = FCell();

// The length of the one cell
const float FCell::CellSize = 200.0F;

// The main constructor. Finds the nearest free cell in the Grid Array for the specified Map Component's owner.
FCell::FCell(const UMapComponent* MapComponent)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	AActor* const ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (IsValid(LevelMap) == false  // The Level Map is valid and is not transient
		&& !ensureMsgf(IS_VALID(ComponentOwner), TEXT("FCell:: The specified actor is not valid")))
	{
		return;
	}

	// ----- Part 0: Locals -----

	const FCell OwnerCell(ComponentOwner->GetActorLocation());  // The owner location
	// Check if the owner already standing on:
	FCells CellsToIterate({OwnerCell,																   // 0: exactly the current his cell
		FCell(OwnerCell.RotateAngleAxis(-1.F).Location.GridSnap(CellSize)).RotateAngleAxis(1.F)});	 // 1: within the radius of one cell
	const int32 InitialCellsNum = CellsToIterate.Num();												   // The number of initial cells
	FCells NonEmptyCells;																			   // all cells of each level actor
	LevelMap->IntersectCellsByTypes(NonEmptyCells, TO_FLAG(~EActorType::Player), true, MapComponent);  //EActorType::Bomb | EActorType::Item | EActorType::Wall | EActorType::Box

	// Pre gameplay locals to find a nearest cell
	const bool bHasNotBegunPlay = !LevelMap->HasActorBegunPlay();  // the game was not started
	float LastFoundEditorLen = MAX_FLT;
	if (bHasNotBegunPlay)
	{
		CellsToIterate.Append(LevelMap->GridCells_);  // union of two sets(initials+all) for finding a nearest cell
	}

	// ----- Part 1:  Cells iteration

	int32 Counter = -1;
	for (const auto& CellIt : CellsToIterate)
	{
		Counter++;
		USingletonLibrary::PrintToLog(ComponentOwner, "FCell(MapComponent)", FString::FromInt(Counter) + ":" + CellIt.Location.ToString());

		if (NonEmptyCells.Contains(CellIt)					   // the cell is not free from other level actors
			&& MapComponent->ActorType != EActorType::Player)  // the player can be placed with other actor
		{
			continue;
		}

		// if the cell was found among initial cells without searching a nearest
		if (Counter < InitialCellsNum				   // is the initial cell
			&& LevelMap->GridCells_.Contains(CellIt))  // is contained on the grid
		{
			*this = CellIt;
			break;
		}

		//	Finding the nearest cell before starts the game
		if (bHasNotBegunPlay				// the game was not started
			&& Counter >= InitialCellsNum)  // if iterated cell is not initial
		{
			const float EditorLenIt = USingletonLibrary::CalculateCellsLength(OwnerCell, CellIt);
			if (EditorLenIt < LastFoundEditorLen)  // Distance closer
			{
				LastFoundEditorLen = EditorLenIt;
				*this = CellIt;
			}
		}

	}  //[Cells Iteration]

	// Checks the cell is contained in the grid and free from other level actors.
	bWasFound = !(*this == ZeroCell) && !NonEmptyCells.Contains(*this);
}

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(FVector Vector)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	bWasFound = IsValid(LevelMap);
	Location.X = FMath::RoundToFloat((Vector.X));
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(bWasFound ? LevelMap->GetActorLocation().Z : Vector.Z);
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(const float& AxisZ) const
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IsValid(LevelMap) == false  //
		|| !ensureAlwaysMsgf(AxisZ != abs(0.f), TEXT("The axis is zero")))
	{
		return *this;
	}

	const FVector Dimensions = this->Location - LevelMap->GetActorLocation();
	const FVector RotatedVector = Dimensions.RotateAngleAxis(LevelMap->GetActorRotation().Yaw, FVector(0, 0, AxisZ));
	return FCell(this->Location + RotatedVector - Dimensions);
}

// Copy another non-zero cell into this one.
FCell& FCell::operator=(const FCell& Other)
{
	if (!(Other == ZeroCell))
	{
		Location = Other.Location;
		bWasFound = Other.bWasFound;
	}
	return *this;
}