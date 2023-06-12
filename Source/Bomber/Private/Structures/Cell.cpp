// Copyright (c) Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(Cell)

const FCell FCell::InvalidCell = FCell(0.f, 0.f, -1.f);
const FCell FCell::ForwardCell = FCell(1.f, 0.f, 0.f);
const FCell FCell::BackwardCell = FCell(-1.f, 0.f, 0.f);
const FCell FCell::RightCell = FCell(0.f, 1.f, 0.f);
const FCell FCell::LeftCell = FCell(0.f, -1.f, 0.f);
const FCells FCell::EmptyCells = FCells{};

// Vector to cell constructor
FCell::FCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(Vector.Z);
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
FCell FCell::RotateCellAroundOrigin(const FCell& InCell, float AxisZ, const FTransform& OriginTransformNoScale)
{
	const FVector Dimensions = InCell.Location - OriginTransformNoScale.GetLocation();
	const float AngleDeg = OriginTransformNoScale.GetRotation().Rotator().Yaw;
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

	const FTransform CellArrayTransformNoScale = GetCellArrayTransformNoScale(InCells);

	FCells RotatedCells = EmptyCells;
	for (const FCell& CellIt : InCells)
	{
		const FCell RotatedCell = RotateCellAroundOrigin(CellIt, AxisZ, CellArrayTransformNoScale);
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
			// Locate the cell relative to the Generated Map
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

// Returns the cell by specified column (X) and row (Y) on given grid if exists, invalid cell otherwise
FCell FCell::GetCellByPositionOnGrid(const FIntPoint& CellPosition, const FCells& InGrid)
{
	const int32 MaxWidth = FMath::FloorToInt32(GetCellArrayWidth(InGrid));
	const int32 CellIndex = CellPosition.Y/*Row*/ * MaxWidth/*ColumnsNum*/ + CellPosition.X/*Column*/;
	const TArray<FCell>& GridArray = InGrid.Array();
	return GridArray.IsValidIndex(CellIndex) ? GridArray[CellIndex] : InvalidCell;
}

// Takes the cell and returns its column (X) and row (Y) position on given grid if exists, -1 otherwise
FIntPoint FCell::GetPositionByCellOnGrid(const FCell& InCell, const FCells& InGrid)
{
	const int32 MaxWidth = GetCellArrayWidth(InGrid);
	const int32 CellIdx = InGrid.Array().IndexOfByPredicate([&InCell](const FCell& CellIt) { return CellIt == InCell; });
	const bool bFound = CellIdx != INDEX_NONE && MaxWidth > 0;
	return FIntPoint(
		bFound ? CellIdx % MaxWidth : INDEX_NONE,  // Column
		bFound ? CellIdx / MaxWidth : INDEX_NONE); // Row
}

// Returns the center column (X) and row (Y) position on given grid
FIntPoint FCell::GetCenterCellPositionOnGrid(const FCells& InGrid)
{
	return FIntPoint(
		FMath::FloorToInt32(GetCellArrayWidth(InGrid) / 2.f),
		FMath::FloorToInt32(GetCellArrayLength(InGrid) / 2.f));
}

// Returns 4 corner cells on given cells grid
FCells FCell::GetCornerCellsOnGrid(const FCells& InGrid)
{
	return FCells
	{
		GetCellByCornerOnGrid(EGridCorner::TopLeft, InGrid),
		GetCellByCornerOnGrid(EGridCorner::TopRight, InGrid),
		GetCellByCornerOnGrid(EGridCorner::BottomLeft, InGrid),
		GetCellByCornerOnGrid(EGridCorner::BottomRight, InGrid)
	};
}

// Returns specified corner cell in given grid
FCell FCell::GetCellByCornerOnGrid(EGridCorner CornerType, const FCells& InGrid)
{
	// +---+---+---+
	// | X |   | X |
	// +---+---+---+
	// |   |   |   |
	// +---+---+---+
	// | X |   | X |
	// +---+---+---+

	constexpr int32 FirstCellIndex = 0;
	const int32 LastColumnIndex = GetCellArrayWidth(InGrid) - 1;
	const int32 LastRowIndex = GetCellArrayLength(InGrid) - 1;

	switch (CornerType)
	{
		case EGridCorner::TopLeft:
			return GetCellByPositionOnGrid(FIntPoint(FirstCellIndex, FirstCellIndex), InGrid);
		case EGridCorner::TopRight:
			return GetCellByPositionOnGrid(FIntPoint(LastColumnIndex, FirstCellIndex), InGrid);
		case EGridCorner::BottomLeft:
			return GetCellByPositionOnGrid(FIntPoint(FirstCellIndex, LastRowIndex), InGrid);
		case EGridCorner::BottomRight:
			return GetCellByPositionOnGrid(FIntPoint(LastColumnIndex, LastRowIndex), InGrid);
		default:
			return InvalidCell;
	}
}

// Scales specified cell maintaining relative distance from the corners of the new grid
FCell FCell::ScaleCellToNewGrid(const FCell& OriginalCell, const FCells& NewCornerCells)
{
	constexpr int32 CornerNums = 4;
	if (!ensureMsgf(NewCornerCells.Num() == CornerNums, TEXT("%s: 'NewCornerCells' has different number than 4"), *FString(__FUNCTION__)))
	{
		return InvalidCell;
	}

	// Use bilinear interpolation to find the new cell
	TArray<float> Weights = {
		(1 - OriginalCell.X()) * (1 - OriginalCell.Y()),
		OriginalCell.X() * (1 - OriginalCell.Y()),
		(1 - OriginalCell.X()) * OriginalCell.Y(),
		OriginalCell.X() * OriginalCell.Y(),
	};

	const TArray<FCell> Corners = NewCornerCells.Array();
	FCell ScaledCell = InvalidCell;
	for (int32 i = 0; i < NewCornerCells.Num(); ++i)
	{
		ScaledCell += Corners[i] * Weights[i];
	}

	return MoveTemp(ScaledCell);
}

// Makes origin transform for given grid
FTransform FCell::GetCellArrayTransform(const FCells& InCells)
{
	FTransform OriginTransform = GetCellArrayTransformNoScale(InCells);
	OriginTransform.SetScale3D(FVector(GetCellArraySize(InCells), 1.f));
	return MoveTemp(OriginTransform);
}

// Makes origin transform for given grid without scale
FTransform FCell::GetCellArrayTransformNoScale(const FCells& InCells)
{
	FTransform OriginTransform = FTransform::Identity;
	OriginTransform.SetLocation(GetCellArrayCenter(InCells));
	OriginTransform.SetRotation(GetCellArrayRotation(InCells).Quaternion());
	return MoveTemp(OriginTransform);
}

// Find the average of an array of vectors
FCell FCell::GetCellArrayCenter(const FCells& Cells)
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

// Makes rotator for given grid its origin
FRotator FCell::GetCellArrayRotation(const FCells& InCells)
{
	if (InCells.Num() < 2)
	{
		return FRotator::ZeroRotator;
	}

	const FCell& FirstCell = *InCells.CreateConstIterator();
	const FCell& SecondCell = *++InCells.CreateConstIterator();
	const FVector Direction = SecondCell - FirstCell;
	FRotator Rotator = Direction.Rotation();
	return MoveTemp(Rotator);
}

// Returns the width (columns X) and the length (rows Y) in specified cells, where each 1 unit means 1 cell
FVector2D FCell::GetCellArraySize(const FCells& InCells)
{
	const float WidthX = GetCellArrayWidth(InCells);
	const float LengthY = GetCellArrayLength(InCells);
	return FVector2D(WidthX, LengthY);
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
