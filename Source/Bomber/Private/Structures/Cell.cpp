// Copyright (c) Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
//---

// The zero cell
const FCell FCell::ZeroCell = FCell();

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(AGeneratedMap::Get().GetCachedTransform().GetLocation().Z);
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(float AxisZ) const
{
	const FTransform& LevelTransform = AGeneratedMap::Get().GetCachedTransform();
	const FVector Dimensions = Location - LevelTransform.GetLocation();
	const float AngleDeg = LevelTransform.GetRotation().Rotator().Yaw;
	const FVector Axis(0, 0, AxisZ);
	const FVector RotatedVector = Dimensions.RotateAngleAxis(AngleDeg, Axis);
	return FCell(Location + RotatedVector - Dimensions);
}

// Sums cells
FCell& FCell::operator+=(const FCell& Other)
{
	Location += Other.Location;
	return *this;
}

// Subtracts a cell from another cell
FCell& FCell::operator-=(const FCell& Other)
{
	Location -= Other.Location;
	return *this;
}

// Calculate the length between two cells
float UCellsUtilsLibrary::GetLengthBetweenCells(const FCell& C1, const FCell& C2)
{
	const float CellSize = GetCellSize();
	if (!CellSize)
	{
		return 0.f;
	}

	return FMath::Abs((C1.Location - C2.Location).Size()) / CellSize;
}

// Find the average of an array of vectors
FCell UCellsUtilsLibrary::GetCellArrayAverage(const FCells& Cells)
{
	FVector Sum = FVector::ZeroVector;
	FVector Average = FVector::ZeroVector;
	const float CellsNum = static_cast<float>(Cells.Num());
	if (CellsNum > 0.f)
	{
		for (const FCell& CellIt : Cells)
		{
			Sum += CellIt.Location;
		}

		Average = Sum / CellsNum;
	}
	FVector2D V(FVector::UpVector);
	return FCell(Average);
}
