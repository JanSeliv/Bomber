// Copyright (c) Yevhenii Selivanov.

#include "Structures/BmrCell.h"

// Bomber
#include "Engine/NetSerialization.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCell)

const FBmrCell FBmrCell::InvalidCell = FBmrCell(0.f, 0.f, -1.f);
const FBmrCell FBmrCell::ForwardCell = FBmrCell(1.f, 0.f, 0.f);
const FBmrCell FBmrCell::BackwardCell = FBmrCell(-1.f, 0.f, 0.f);
const FBmrCell FBmrCell::RightCell = FBmrCell(0.f, 1.f, 0.f);
const FBmrCell FBmrCell::LeftCell = FBmrCell(0.f, -1.f, 0.f);
const FBmrCells FBmrCell::EmptyCells = FBmrCells{};
const FBmrCellsArr FBmrCell::EmptyCellsArr = FBmrCellsArr{};

// Vector to cell constructor
FBmrCell::FBmrCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(Vector.Z);
}

FBmrCell::FBmrCell(const FVector_NetQuantize& Vector)
    : FBmrCell(FVector(Vector))
{
}

// Floats to cell constructor
FBmrCell::FBmrCell(float X, float Y, float Z)
{
	Location.X = FMath::RoundToFloat(X);
	Location.Y = FMath::RoundToFloat(Y);
	Location.Z = FMath::RoundToFloat(Z);
}

// Doubles to cell constructor
FBmrCell::FBmrCell(double X, double Y, double Z)
{
	Location.X = FMath::RoundToDouble(X);
	Location.Y = FMath::RoundToDouble(Y);
	Location.Z = FMath::RoundToDouble(Z);
}

// Equal operator for vectors to directly copy its value to the cell
FBmrCell& FBmrCell::operator=(const FVector& Vector)
{
	Location = Vector;
	return *this;
}

FBmrCell& FBmrCell::operator=(const FVector_NetQuantize& Vector)
{
	Location = FVector(Vector);
	return *this;
}

/*********************************************************************************************
 * Rotation
 ********************************************************************************************* */

// Gets a copy of given cell rotated around given transform to the same yaw degree
FBmrCell FBmrCell::RotateCellAroundOrigin(const FBmrCell& InCell, float AxisZ, const FTransform& OriginTransformNoScale)
{
	const FVector Dimensions = InCell.Location - OriginTransformNoScale.GetLocation();
	const float AngleDeg = OriginTransformNoScale.GetRotation().Rotator().Yaw;
	const FVector Axis(FVector2D::ZeroVector, AxisZ);
	const FVector RotatedVector = Dimensions.RotateAngleAxis(AngleDeg, Axis);
	FBmrCell RotatedCell(InCell.Location + RotatedVector - Dimensions);
	return MoveTemp(RotatedCell);
}

// Allows rotate or unrotated given grid around its origin
FBmrCells FBmrCell::RotateCellArray(float AxisZ, const FBmrCells& InCells)
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

	FBmrCells RotatedCells = EmptyCells;
	for (const FBmrCell& CellIt : InCells)
	{
		const FBmrCell RotatedCell = RotateCellAroundOrigin(CellIt, AxisZ, CellArrayTransformNoScale);
		FBmrCell SnappedCell = SnapCell(RotatedCell);
		RotatedCells.Emplace(MoveTemp(SnappedCell));
	}

	return MoveTemp(RotatedCells);
}

/*********************************************************************************************
 * Grid
 ********************************************************************************************* */

// Constructs and returns new grid from given transform
FBmrCells FBmrCell::MakeCellGridByTransform(const FTransform& OriginTransform)
{
	const FVector LevelLocation = OriginTransform.GetLocation();
	const FVector LevelScale = OriginTransform.GetScale3D();
	const FIntPoint LevelSize(LevelScale.X, LevelScale.Y);

	FBmrCells GridCells;
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
			const FBmrCell SnappedCell = SnapCell(FoundVector);
			// Cell was found, add rotated cell to the array
			FBmrCell RotatedCell = RotateCellAroundOrigin(SnappedCell, 1.f, OriginTransform);

			GridCells.Emplace(MoveTemp(RotatedCell));
		}
	}

	return MoveTemp(GridCells);
}

// Returns the cell by specified column (X) and row (Y) on given grid if exists, invalid cell otherwise
FBmrCell FBmrCell::GetCellByPositionOnGrid(const FIntPoint& CellPosition, const FBmrCells& InGrid)
{
	const int32 MaxWidth = FMath::FloorToInt32(GetCellArrayWidth(InGrid));
	return GetCellByPositionOnGrid(CellPosition, InGrid.Array(), MaxWidth);
}

// More optimized version of GetCellByPositionOnGrid which requires the Grid Width to be passed
FBmrCell FBmrCell::GetCellByPositionOnGrid(const FIntPoint& CellPosition, const FBmrCellsArr& InGrid, int32 GridWidth)
{
	const int32 CellIndex = CellPosition.Y /*Row*/ * GridWidth /*ColumnsNum*/ + CellPosition.X /*Column*/;
	return InGrid.IsValidIndex(CellIndex) ? InGrid[CellIndex] : InvalidCell;
}

// Takes the cell and returns its column (X) and row (Y) position on given grid if exists, -1 otherwise
FIntPoint FBmrCell::GetPositionByCellOnGrid(const FBmrCell& InCell, const FBmrCells& InGrid)
{
	const int32 MaxWidth = GetCellArrayWidth(InGrid);
	return GetPositionByCellOnGrid(InCell, InGrid.Array(), MaxWidth);
}

// More optimized version of GetPositionByCellOnGrid which requires the Grid Size to be passed
FIntPoint FBmrCell::GetPositionByCellOnGrid(const FBmrCell& InCell, const FBmrCellsArr& InGrid, int32 GridWidth)
{
	if (GridWidth <= 0)
	{
		return FIntPoint(-1, -1);
	}

	const int32 Index = InGrid.Find(InCell);
	if (Index == INDEX_NONE)
	{
		return FIntPoint(-1, -1);
	}

	const int32 PositionY = Index / GridWidth;
	const int32 PositionX = Index % GridWidth;

	return FIntPoint(PositionX, PositionY);
}

// Returns a map of cells to their positions on the grid
TMap<FBmrCell, FIntPoint> FBmrCell::GetPositionsByCellsOnGrid(const FBmrCellsArr& InGrid, int32 GridWidth)
{
	TMap<FBmrCell, FIntPoint> AllCellPositions;
	for (const FBmrCell& It : InGrid)
	{
		const FIntPoint Position = GetPositionByCellOnGrid(It, InGrid, GridWidth);
		if (Position.X >= 0 && Position.Y >= 0)
		{
			AllCellPositions.Emplace(It, Position);
		}
	}

	return AllCellPositions;
}

// Returns the center column (X) and row (Y) position on given grid
FIntPoint FBmrCell::GetCenterCellPositionOnGrid(const FBmrCells& InGrid)
{
	return FIntPoint(
	    FMath::FloorToInt32(GetCellArrayWidth(InGrid) / 2.f),
	    FMath::FloorToInt32(GetCellArrayLength(InGrid) / 2.f));
}

// Returns 4 corner cells on given cells grid
FBmrCells FBmrCell::GetCornerCellsOnGrid(const FBmrCells& InGrid)
{
	return FBmrCells{
	    GetCellByCornerOnGrid(EBmrGridCorner::TopLeft, InGrid),
	    GetCellByCornerOnGrid(EBmrGridCorner::TopRight, InGrid),
	    GetCellByCornerOnGrid(EBmrGridCorner::BottomLeft, InGrid),
	    GetCellByCornerOnGrid(EBmrGridCorner::BottomRight, InGrid)};
}

// Returns specified corner cell in given grid
FBmrCell FBmrCell::GetCellByCornerOnGrid(EBmrGridCorner CornerType, const FBmrCells& InGrid)
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
		case EBmrGridCorner::TopLeft:
			return GetCellByPositionOnGrid(FIntPoint(FirstCellIndex, FirstCellIndex), InGrid);
		case EBmrGridCorner::TopRight:
			return GetCellByPositionOnGrid(FIntPoint(LastColumnIndex, FirstCellIndex), InGrid);
		case EBmrGridCorner::BottomLeft:
			return GetCellByPositionOnGrid(FIntPoint(FirstCellIndex, LastRowIndex), InGrid);
		case EBmrGridCorner::BottomRight:
			return GetCellByPositionOnGrid(FIntPoint(LastColumnIndex, LastRowIndex), InGrid);
		default:
			return InvalidCell;
	}
}

// Scales specified cell maintaining relative distance from the corners of the new grid
FBmrCell FBmrCell::ScaleCellToNewGrid(const FBmrCell& OriginalCell, const FBmrCells& NewCornerCells)
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

	const TArray<FBmrCell> Corners = NewCornerCells.Array();
	FBmrCell ScaledCell = InvalidCell;
	for (int32 i = 0; i < NewCornerCells.Num(); ++i)
	{
		ScaledCell += Corners[i] * Weights[i];
	}

	return MoveTemp(ScaledCell);
}

// Makes origin transform for given grid
FTransform FBmrCell::GetCellArrayTransform(const FBmrCells& InCells)
{
	FTransform OriginTransform = GetCellArrayTransformNoScale(InCells);
	OriginTransform.SetScale3D(FVector(GetCellArraySize(InCells), 1.f));
	return MoveTemp(OriginTransform);
}

// Makes origin transform for given grid without scale
FTransform FBmrCell::GetCellArrayTransformNoScale(const FBmrCells& InCells)
{
	FTransform OriginTransform = FTransform::Identity;
	OriginTransform.SetLocation(GetCellArrayCenter(InCells));
	OriginTransform.SetRotation(GetCellArrayRotation(InCells).Quaternion());
	return MoveTemp(OriginTransform);
}

// Find the average of an array of vectors
FBmrCell FBmrCell::GetCellArrayCenter(const FBmrCells& Cells)
{
	FVector Sum = FVector::ZeroVector;
	FVector Average = FVector::ZeroVector;
	const float CellsNum = static_cast<float>(Cells.Num());
	if (CellsNum > 0.f)
	{
		for (const FBmrCell& CellIt : Cells)
		{
			Sum += CellIt.Location;
		}

		Average = Sum / CellsNum;
	}
	return FBmrCell(Average);
}

// Makes rotator for given grid its origin
FRotator FBmrCell::GetCellArrayRotation(const FBmrCells& InCells)
{
	if (InCells.Num() < 2)
	{
		return FRotator::ZeroRotator;
	}

	const FBmrCell& FirstCell = *InCells.CreateConstIterator();
	const FBmrCell& SecondCell = *++InCells.CreateConstIterator();
	const FVector Direction = SecondCell - FirstCell;
	FRotator Rotator = Direction.Rotation();
	return MoveTemp(Rotator);
}

// Returns the width (columns X) and the length (rows Y) in specified cells, where each 1 unit means 1 cell
FVector2D FBmrCell::GetCellArraySize(const FBmrCells& InCells)
{
	const float WidthX = GetCellArrayWidth(InCells);
	const float LengthY = GetCellArrayLength(InCells);
	return FVector2D(WidthX, LengthY);
}

// Returns number of columns (X) in specified cells array, where each 1 unit means 1 cell
float FBmrCell::GetCellArrayWidth(const FBmrCells& InCells)
{
	if (InCells.IsEmpty())
	{
		return 0.f;
	}

	const FBmrCells UnrotatedCells = RotateCellArray(-1.f, InCells);
	const FBox CellsBox(CellsToVectors(UnrotatedCells));
	return CellsBox.GetSize().X / CellSize + 1.f;
}

// Returns number of rows (Y) in specified cells array, where each 1 unit means 1 cell
float FBmrCell::GetCellArrayLength(const FBmrCells& InCells)
{
	if (InCells.IsEmpty())
	{
		return 0.f;
	}

	const FBmrCells UnrotatedCells = RotateCellArray(-1.f, InCells);
	const FBox CellsBox(CellsToVectors(UnrotatedCells));
	return CellsBox.GetSize().Y / CellSize + 1.f;
}

// Finds the closest cell to the given cell within array of cells
FBmrCell FBmrCell::GetCellArrayNearest(const FBmrCells& InCells, const FBmrCell& CellToCheck)
{
	//-----------------
	// +-----+-----+   |
	// |  1  |  2  |   | 1-4: are given values in 'Cells' array
	// +-----+-----+   |
	// | [3] |  4  |   | [3]: is the found nearest cell.
	// +-----+-----+   |
	//    X            | X: is given 'CellToCheck'
	//-----------------

	if (!InCells.Num())
	{
		return InvalidCell;
	}

	FBmrCell NearestCell = InvalidCell;
	static constexpr float MaxFloat = TNumericLimits<float>::Max();
	float NearestDistance = MaxFloat;

	for (const FBmrCell& CellIt : InCells)
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

// Keeps cells within range of the StartingCell and avoids barriers
FBmrCells FBmrCell::FilterCellsByBounds(const FBmrCells& ActiveCells, const FBmrCells& BoundaryCells, const FBmrCell& StartingCell)
{
	// Input Grid:            Resulting Grid:
	// +---+---+---+---+      +---+---+---+---+
	// | + | B | + | + |      | - | B | - | + |
	// +---+---+---+---+      +---+---+---+---+
	// | B | B | B | + |      | B | B | B | + |
	// +---+---+---+---+      +---+---+---+---+
	// | + | B | + | + |      | - | B | + | + |
	// +---+---+---+---+      +---+---+---+---+
	// | + | + | + |[S]|      | + | + | + |[S]|
	// +---+---+---+---+      +---+---+---+---+
	//
	// B are cells in BoundaryCells (act as barriers)
	// S is Starting cell (reference point for trimming)
	// + are Active Cells to be processed and filtered
	// - are cells that are filtered out

	FBmrCells ResultCells = EmptyCells;

	// First iteration, determine all cells within line of sight
	FBmrCells VisibleCells = EmptyCells;
	for (const FBmrCell& ActiveCellIt : ActiveCells)
	{
		if (BoundaryCells.Contains(ActiveCellIt))
		{
			// Active cell is a boundary cell, skip it completely
			continue;
		}

		if (CanCellSeeTarget(StartingCell, ActiveCellIt, BoundaryCells))
		{
			// The cell is withing the bounds, add it to further processing
			VisibleCells.Add(ActiveCellIt);
			continue;
		}

		// By default, all not visible cells are treated as within the bounds (as they are not arounded by any boundary cells)
		ResultCells.Add(ActiveCellIt);
	}

	// Second iteration: Process the visible cells by comparing distances to StartingCell
	for (const FBmrCell& VisibleCellIt : VisibleCells)
	{
		bool bKeepCell = true;
		const float DistanceToVisible = FVector::Dist(StartingCell.Location, VisibleCellIt.Location);

		for (const FBmrCell& BoundaryCellIt : BoundaryCells)
		{
			const float DistanceToBoundary = FVector::Dist(StartingCell.Location, BoundaryCellIt.Location);

			// Exclude the visible cell if it's farther than (or equal to) any boundary cell
			if (DistanceToVisible >= DistanceToBoundary
			    || DistanceToVisible < CellSize)
			{
				bKeepCell = false;
				break;
			}
		}

		if (bKeepCell)
		{
			ResultCells.Add(VisibleCellIt);
		}
	}

	return ResultCells;
}

// Returns true if Starting Cell is in the direction of any of the Target Cells within the given angle
bool FBmrCell::CanCellSeeTarget(const FBmrCell& StartingCell, const FBmrCell& TargetCell, const FBmrCells& AllVisibleCells, float MaxAngleDegrees /* = 40.f*/)
{
	/**     .  .  .  T  .  .  .
	 *             \ | /
	 *              \|/
	 *               S
	 *
	 * S - StartingCell
	 * T - TargetCells */

	const FVector DirectionTarget = (TargetCell.Location - StartingCell.Location).GetSafeNormal();
	if (DirectionTarget.IsNearlyZero())
	{
		return false;
	}

	for (const FBmrCell& VisibleCell : AllVisibleCells)
	{
		// Compute direction from StartingCell to the current visible cell
		const FVector DirectionVisible = (VisibleCell.Location - StartingCell.Location).GetSafeNormal();
		if (DirectionVisible.IsNearlyZero())
		{
			continue;
		}

		// Dot product tells us how aligned the two directions are
		const float Dot = FVector::DotProduct(DirectionTarget, DirectionVisible);
		if (Dot <= 0.0f)
		{
			// Directions are more than 90 degrees apart or invalid
			continue;
		}

		// Convert the dot product result into an angle (in degrees)
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
		if (AngleDegrees <= MaxAngleDegrees)
		{
			// As soon as we find a visible cell meeting the criteria, we can return true
			return true;
		}
	}

	return false;
}

// Fisher-Yates shuffle algorithm to reorder the array elements
void FBmrCell::RandShuffle(FBmrCellsArr& InOutArray)
{
	const int32 LastIndex = InOutArray.Num() - 1;
	for (int32 Index = LastIndex; Index > 0; --Index)
	{
		const int32 RandomIndex = FMath::RandRange(0, Index);

		// Swap the element at the current index with a randomly selected element
		// from the unshuffled part of the array
		InOutArray.Swap(Index, RandomIndex);
	}
}

// Extracts the top-left quarter of the grid
FBmrCellsArr FBmrCell::GetTopLeftQuarterOnGrid(const FBmrCellsArr& InGrid, const FIntPoint& MapScale)
{
	FBmrCellsArr QuarterCells;
	static constexpr int32 HalfAxisScaleDivider = 2;
	const FIntPoint QuarterScale(MapScale.X / HalfAxisScaleDivider + 1, MapScale.Y / HalfAxisScaleDivider + 1);

	for (int32 IndexY = 0; IndexY < QuarterScale.Y; ++IndexY)
	{
		for (int32 IndexX = 0; IndexX < QuarterScale.X; ++IndexX)
		{
			const int32 CellIndex = IndexY * MapScale.X + IndexX;
			if (InGrid.IsValidIndex(CellIndex))
			{
				QuarterCells.Add(InGrid[CellIndex]);
			}
		}
	}
	return QuarterCells;
}

/*********************************************************************************************
 * Math operators
 ********************************************************************************************* */

// Sums cells
FBmrCell& FBmrCell::operator+=(const FBmrCell& Other)
{
	Location += Other.Location;
	return *this;
}

// Subtracts a cell from another cell
FBmrCell& FBmrCell::operator-=(const FBmrCell& Other)
{
	Location -= Other.Location;
	return *this;
}

/*********************************************************************************************
 * Conversion
 ********************************************************************************************* */

// Vector operator to return cell location
FBmrCell::operator FVector_NetQuantize() const
{
	return FVector_NetQuantize(Location);
}

// Returns the cell direction by its enum.
const FBmrCell& FBmrCell::GetCellDirection(EBmrCellDirection CellDirection)
{
	switch (CellDirection)
	{
		case EBmrCellDirection::Forward:
			return ForwardCell;
		case EBmrCellDirection::Backward:
			return BackwardCell;
		case EBmrCellDirection::Right:
			return RightCell;
		case EBmrCellDirection::Left:
			return LeftCell;
		default:
			return InvalidCell;
	}
}

// Returns the enum direction by its cell
EBmrCellDirection FBmrCell::GetCellDirection(const FBmrCell& CellDirection)
{
	if (CellDirection == ForwardCell)
	{
		return EBmrCellDirection::Forward;
	}
	if (CellDirection == BackwardCell)
	{
		return EBmrCellDirection::Backward;
	}
	if (CellDirection == RightCell)
	{
		return EBmrCellDirection::Right;
	}
	if (CellDirection == LeftCell)
	{
		return EBmrCellDirection::Left;
	}
	return EBmrCellDirection::None;
}

// Converts set of cells to array of vectors
TArray<FVector> FBmrCell::CellsToVectors(const FBmrCells& Cells)
{
	TArray<FVector> Vectors;
	Vectors.Reserve(Cells.Num());
	for (const FBmrCell& Cell : Cells)
	{
		Vectors.Add(Cell.Location);
	}
	return Vectors;
}

// Converts array of vectors to to set of cells
FBmrCells FBmrCell::VectorsToCells(const TArray<FVector>& Vectors)
{
	FBmrCells Cells;
	Cells.Reserve(Vectors.Num());
	for (const FVector& Vector : Vectors)
	{
		Cells.Add(FBmrCell(Vector));
	}
	return MoveTemp(Cells);
}