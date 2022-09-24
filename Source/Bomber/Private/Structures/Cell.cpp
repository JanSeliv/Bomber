// Copyright (c) Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include "GeneratedMap.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"

const FCell FCell::InvalidCell = FVector::DownVector;
const FCell FCell::ForwardCell = FVector::ForwardVector;
const FCell FCell::BackwardCell = FVector::BackwardVector;
const FCell FCell::RightCell = FVector::RightVector;
const FCell FCell::LeftCell = FVector::LeftVector;
const FCells FCell::EmptyCells = FCells{};

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(GEngine && GEngine->GameSingleton ? UCellsUtilsLibrary::GetCellHeightLocation() : Vector.Z);
}

// Equal operator for vectors to directly copy its value to the cell
FCell& FCell::operator=(const FVector& Vector)
{
	Location = Vector;
	return *this;
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(float AxisZ) const
{
	const FTransform& LevelTransform = AGeneratedMap::Get().GetCachedTransform();
	const FVector Dimensions = Location - LevelTransform.GetLocation();
	const float AngleDeg = LevelTransform.GetRotation().Rotator().Yaw;
	const FVector Axis(FVector2D::ZeroVector, AxisZ);
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

// Find the average of an array of vectors
FCell FCell::GetCellArrayAverage(const FCells& Cells)
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
	return FCell(Average);
}

// Returns the cell direction by its enum.
const FCell& FCell::GetCellDirection(ECellDirection CellDirection)
{
	switch (CellDirection)
	{
		case ECellDirection::Forward:
			return ForwardCell;
		case ECellDirection::Backward:
			return BackwardCell;
		case ECellDirection::Right:
			return RightCell;
		case ECellDirection::Left:
			return LeftCell;
		default:
			return InvalidCell;
	}
}

// Returns the enum direction by its cell
ECellDirection FCell::GetCellDirection(const FCell& CellDirection)
{
	if (CellDirection == ForwardCell)
	{
		return ECellDirection::Forward;
	}
	if (CellDirection == BackwardCell)
	{
		return ECellDirection::Backward;
	}
	if (CellDirection == RightCell)
	{
		return ECellDirection::Right;
	}
	if (CellDirection == LeftCell)
	{
		return ECellDirection::Left;
	}
	return ECellDirection::None;
}
