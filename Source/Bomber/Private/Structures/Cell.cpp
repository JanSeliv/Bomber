// Copyright (c) Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include "UtilityLibraries/CellsUtilsLibrary.h"

const FCell FCell::InvalidCell = FVector::DownVector;
const FCell FCell::ForwardCell = FVector::ForwardVector;
const FCell FCell::BackwardCell = FVector::BackwardVector;
const FCell FCell::RightCell = FVector::RightVector;
const FCell FCell::LeftCell = FVector::LeftVector;
const FCells FCell::EmptyCells = FCells{};

// Vector to cell constructor
FCell::FCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(GEngine && GEngine->GameSingleton ? UCellsUtilsLibrary::GetCellHeightLocation() : Vector.Z);
}

// Floats to cell constructor
FCell::FCell(float X, float Y, float Z)
{
	Location.X = FMath::RoundToFloat(X);
	Location.Y = FMath::RoundToFloat(Y);
	Location.Z = FMath::RoundToFloat(Z);
}

// Doubles to cell constructor
FCell::FCell(double X, double Y, double Z)
{
	Location.X = FMath::RoundToDouble(X);
	Location.Y = FMath::RoundToDouble(Y);
	Location.Z = FMath::RoundToDouble(Z);
}

// Equal operator for vectors to directly copy its value to the cell
FCell& FCell::operator=(const FVector& Vector)
{
	Location = Vector;
	return *this;
}

// Gets a copy of given cell rotated around given transform to the same yaw degree
FCell FCell::RotateCellAroundOrigin(const FCell& InCell, float AxisZ, const FTransform& OriginTransform)
{
	const FVector Dimensions = InCell.Location - OriginTransform.GetLocation();
	const float AngleDeg = OriginTransform.GetRotation().Rotator().Yaw;
	const FVector Axis(FVector2D::ZeroVector, AxisZ);
	const FVector RotatedVector = Dimensions.RotateAngleAxis(AngleDeg, Axis);
	FCell RotatedCell(InCell.Location + RotatedVector - Dimensions);
	return MoveTemp(RotatedCell);
}

// Finds the closest cell to the given cell within array of cells
FCell FCell::GetCellArrayNearest(const FCells& Cells, const FCell& CellToCheck)
{
	//-----------------
	// +-----+-----+   |
	// |  1  |  2  |   | 1-4: are given values in 'Cells' array
	// +-----+-----+   |
	// | [3] |  4  |   | [3]: is the found nearest cell.
	// +-----+-----+   |
	//    X            | X: is given 'CellToCheck'
	//-----------------

	if (!Cells.Num())
	{
		return InvalidCell;
	}

	FCell NearestCell = InvalidCell;
	static constexpr float MaxFloat = TNumericLimits<float>::Max();
	float NearestDistance = MaxFloat;

	for (const FCell& CellIt : Cells)
	{
		const float DistanceIt = Distance<float>(CellIt, CellToCheck);
		if (DistanceIt < NearestDistance)
		{
			NearestCell = CellIt;
			NearestDistance = DistanceIt;
		}
	}

	return NearestCell;
}

// Returns number of columns (X) in specified cells array, where each 1 unit means 1 cell
float FCell::GetCellArrayWidth(const FCells& InCells)
{
	const FCells UnrotatedCells = RotateCellArray(-1.f, InCells);
	const FBox CellsBox(CellsToVectors(UnrotatedCells));
	return CellsBox.GetSize().X / CellSize + 1.f;
}

// Returns number of rows (Y) in specified cells array, where each 1 unit means 1 cell
float FCell::GetCellArrayLength(const FCells& InCells)
{
	const FCells UnrotatedCells = RotateCellArray(-1.f, InCells);
	const FBox CellsBox(CellsToVectors(UnrotatedCells));
	return CellsBox.GetSize().Y / CellSize + 1.f;
}

// Allows rotate or unrotated given grid around its origin
FCells FCell::RotateCellArray(float AxisZ, const FCells& InCells)
{
	// In:rotated cells		Out:unrotated cells
	//     /\     				 __ __ __ __
	//    /   \   				|			|
	//   /      \ 				|			|
	//  /        /				|			|
	// /        / 				|			|
	// \       /  				|			|
	//   \    /   				|__ __ __ __|
	//     \ /

	if (InCells.IsEmpty())
	{
		return EmptyCells;
	}

	const FTransform CellGridTransform = GetCellArrayTransform(InCells);

	FCells RotatedCells = EmptyCells;
	for (const FCell& CellIt : InCells)
	{
		const FCell RotatedCell = RotateCellAroundOrigin(CellIt, AxisZ, CellGridTransform);
		FCell SnappedCell = SnapCell(RotatedCell);
		RotatedCells.Emplace(MoveTemp(SnappedCell));
	}

	return MoveTemp(RotatedCells);
}

// Constructs and returns new grid from given transform
FCells FCell::MakeCellGridByTransform(const FTransform& OriginTransform)
{
	const FVector LevelLocation = OriginTransform.GetLocation();
	const FVector LevelScale = OriginTransform.GetScale3D();
	const FIntPoint LevelSize(LevelScale.X, LevelScale.Y);

	FCells GridCells;
	GridCells.Reserve(LevelSize.X * LevelSize.Y);
	for (int32 Y = 0; Y < LevelSize.Y; ++Y)
	{
		for (int32 X = 0; X < LevelSize.X; ++X)
		{
			FVector FoundVector(X, Y, 0.f);
			// Calculate a length of iteration cell
			FoundVector *= CellSize;
			// Locate the cell relative to the Level Map
			FoundVector += LevelLocation;
			// Subtract the deviation from the center
			FoundVector -= (LevelScale / 2.f) * CellSize;
			// Snap to the cell
			const FCell SnappedCell = SnapCell(FoundVector);
			// Cell was found, add rotated cell to the array
			FCell RotatedCell = RotateCellAroundOrigin(SnappedCell, 1.f, OriginTransform);

			GridCells.Emplace(MoveTemp(RotatedCell));
		}
	}

	return MoveTemp(GridCells);
}

// Makes origin transform for given grid
FTransform FCell::GetCellArrayTransform(const FCells& InCells)
{
	FTransform OriginTransform = FTransform::Identity;
	OriginTransform.SetLocation(GetCellArrayAverage(InCells));
	OriginTransform.SetRotation(GetCellArrayRotator(InCells).Quaternion());
	return MoveTemp(OriginTransform);
}

// Makes rotator for given grid its origin
FRotator FCell::GetCellArrayRotator(const FCells& InCells)
{
	if (InCells.Num() < 2)
	{
		return FRotator::ZeroRotator;
	}

	const FCell& FirstCell = *InCells.CreateConstIterator();
	const FCell& SecondCell = *++InCells.CreateConstIterator();
	const FVector Direction = SecondCell - FirstCell;
	return Direction.Rotation();
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

// Converts set of cells to array of vectors
TArray<FVector> FCell::CellsToVectors(const FCells& Cells)
{
	TArray<FVector> Vectors;
	Vectors.Reserve(Cells.Num());
	for (const FCell& Cell : Cells)
	{
		Vectors.Add(Cell.Location);
	}
	return Vectors;
}

// Converts array of vectors to to set of cells
FCells FCell::VectorsToCells(const TArray<FVector>& Vectors)
{
	FCells Cells;
	Cells.Reserve(Vectors.Num());
	for (const FVector& Vector : Vectors)
	{
		Cells.Add(FCell(Vector));
	}
	return MoveTemp(Cells);
}

// Gets a copy of given cell snapped to a grid
FCell FCell::SnapRotatedCell(const FCell& InCell, const FTransform& GridOriginTransform)
{
	const FCell UnrotatedCell = RotateCellAroundOrigin(InCell, -1.f, GridOriginTransform);
	const FCell SnappedCell = SnapCell(UnrotatedCell);
	FCell RotatedCell = RotateCellAroundOrigin(SnappedCell, 1.f, GridOriginTransform);
	return MoveTemp(RotatedCell);
}
