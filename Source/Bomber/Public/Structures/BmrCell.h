// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "BmrCell.generated.h"

struct FBmrCell;
struct FVector_NetQuantize;

/** Typedef to allow for some nicer looking sets of cells. */
typedef TSet<FBmrCell> FBmrCells;
typedef TArray<FBmrCell> FBmrCellsArr;

/**
 * Represents one of direction of a cell.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBmrCellDirection : uint8
{
	None = 0,
	Forward = 1 << 0,
	Backward = 1 << 1,
	Right = 1 << 2,
	Left = 1 << 3,
	All = Forward | Backward | Right | Left
};

ENUM_CLASS_FLAGS(EBmrCellDirection);
using ECD = EBmrCellDirection;

/**
 * Represents corners on the grid
 */
UENUM(BlueprintType)
enum class EBmrGridCorner : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

/**
 * The structure that contains a location of an one cell (tile) on a grid of the Generated Map.
 * X is the column index, Y is the row index on the grid.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Bomber.BmrCellUtilsLibrary.MakeCell", HasNativeBreak = "/Script/Bomber.BmrCellUtilsLibrary.BreakCell"))
struct BOMBER_API FBmrCell
{
	GENERATED_BODY()

	static const FBmrCell InvalidCell;
	static const FBmrCell ForwardCell;
	static const FBmrCell BackwardCell;
	static const FBmrCell RightCell;
	static const FBmrCell LeftCell;
	static const FBmrCells EmptyCells;
	static const FBmrCellsArr EmptyCellsArr;

	/** The length of the one cell */
	static constexpr float CellSize = 200.f;

	/** Always holds the free cell's FVector-coordinate.
	 * If it is not empty or not found, holds the last succeeded due to copy operator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]", meta = (ShowOnlyInnerProperties))
	FVector Location = FVector::DownVector;

	/** Default constructor. */
	FBmrCell() = default;

	/** Vector to cell constructor. */
	FBmrCell(const FVector& Vector);
	FBmrCell(const FVector_NetQuantize& Vector);

	/** Floats to cell constructor. */
	explicit FBmrCell(float X, float Y, float Z);

	/** Doubles to cell constructor. */
	explicit FBmrCell(double X, double Y, double Z);

	/** Equal operator for vectors to directly copy its value to the cell. */
	FBmrCell& operator=(const FVector& Vector);
	FBmrCell& operator=(const FVector_NetQuantize& Vector);

	/** Returns Cell's X component. */
	float X() const { return Location.X; }

	/** Returns Cell's Y component. */
	float Y() const { return Location.Y; }

	/** Returns Cell's Z component. */
	float Z() const { return Location.Z; }

	/** Comparing with uninitialized Invalid Cell. */
	FORCEINLINE bool IsInvalidCell() const { return *this == InvalidCell; }

	/** Check is valid this cell. */
	FORCEINLINE bool IsValid() const { return *this != InvalidCell; }

	/*********************************************************************************************
	 * Rotation
	 ********************************************************************************************* */
public:
	/** Gets a copy of given cell rotated around given transform to the same yaw degree.
	 * @param InCell - The cell to rotate.
	 * @param AxisZ The Z param of the axis to rotate around.
	 * @param OriginTransformNoScale The transform of the origin of the rotation. */
	static FBmrCell RotateCellAroundOrigin(const FBmrCell& InCell, float AxisZ, const FTransform& OriginTransformNoScale);

	/** Allows rotate or unrotated given grid around its origin. */
	static FBmrCells RotateCellArray(float AxisZ, const FBmrCells& InCells);

	/*********************************************************************************************
	 * Grid (Array of cells)
	 ********************************************************************************************* */
public:
	/** Constructs and returns new grid from given transform.
	 * @param OriginTransform its location and rotation is the center of new grid, its scale-X is number of columns, scale-Y is number of rows. */
	static TSet<FBmrCell> MakeCellGridByTransform(const FTransform& OriginTransform);

	/** Returns the cell by specified column (X) and row (Y) on given grid if exists, invalid cell otherwise. */
	static FBmrCell GetCellByPositionOnGrid(const FIntPoint& CellPosition, const FBmrCells& InGrid);

	/** More optimized version of GetCellByPositionOnGrid which requires the Grid Width to be passed. */
	static FBmrCell GetCellByPositionOnGrid(const FIntPoint& CellPosition, const FBmrCellsArr& InGrid, int32 GridWidth);

	/** Takes the cell and returns its column (X) and row (Y) position on given grid if exists, -1 otherwise. */
	static FIntPoint GetPositionByCellOnGrid(const FBmrCell& InCell, const FBmrCells& InGrid);

	/** More optimized version of GetPositionByCellOnGrid which requires the Grid Width to be passed. */
	static FIntPoint GetPositionByCellOnGrid(const FBmrCell& InCell, const FBmrCellsArr& InGrid, int32 GridWidth);

	/** Returns a map of cells to their positions on the grid. */
	static TMap<FBmrCell, FIntPoint> GetPositionsByCellsOnGrid(const FBmrCellsArr& InGrid, int32 GridWidth);

	/** Returns the center column (X) and row (Y) position on given grid.
	 * E.g: for grid with 5 rows and 5 columns, the center cell will be (2,2). */
	static FIntPoint GetCenterCellPositionOnGrid(const FBmrCells& InGrid);

	/** Returns 4 corner cells on given cells grid. */
	static FBmrCells GetCornerCellsOnGrid(const FBmrCells& InGrid);

	/** Returns specified corner cell in given grid. */
	static FBmrCell GetCellByCornerOnGrid(EBmrGridCorner CornerType, const FBmrCells& InGrid);

	/** Scales specified cell maintaining relative distance from the corners of the new grid.
	 * @param OriginalCell The cell to scale.
	 * @param NewCornerCells The new corner cells to scale to.
	 * @return scaled cell, is not aligned to any existed cell, make sure to snap it to the grid. */
	static FBmrCell ScaleCellToNewGrid(const FBmrCell& OriginalCell, const FBmrCells& NewCornerCells);

	/** Extracts first cell from specified cells set.*/
	static FORCEINLINE FBmrCell GetFirstCellInSet(const FBmrCells& InCells) { return !InCells.IsEmpty() ? InCells.Array()[0] : InvalidCell; }

	/** Gets a copy of given cell snapped its location to a grid while it does not respect rotated grids. */
	static FORCEINLINE FBmrCell SnapCell(const FBmrCell& InCell) { return InCell.Location.GridSnap(CellSize); }

	/** Makes origin transform for given grid. */
	static FTransform GetCellArrayTransform(const FBmrCells& InCells);
	static FTransform GetCellArrayTransformNoScale(const FBmrCells& InCells);

	/** Find the average of an set of cells. */
	static FBmrCell GetCellArrayCenter(const FBmrCells& Cells);

	/** Makes rotator for given grid its origin. */
	static FRotator GetCellArrayRotation(const FBmrCells& InCells);

	/** Returns the width (columns X) and the length (rows Y) in specified cells, where each 1 unit means 1 cell.
	 * E.g: if given cells are corner cells on 7x9 level, it will return 7 columns (X) and 9 rows (Y) columns respectively. */
	static FVector2D GetCellArraySize(const FBmrCells& InCells);
	static float GetCellArrayWidth(const FBmrCells& InCells);
	static float GetCellArrayLength(const FBmrCells& InCells);

	/** Finds the closest cell to the given cell within array of cells.
	 * @param InCells The array of cells to search in.
	 * @param CellToCheck The start position of the cell to check. */
	static FBmrCell GetCellArrayNearest(const FBmrCells& InCells, const FBmrCell& CellToCheck);

	/** Keeps cells within range of the StartingCell and avoids barriers.
	 * E.g: might be useful to exclude all explosions cells and those that are out of explosions, so the bot (Starting Cell) will not attempt to go through explosions.
	 * @param ActiveCells The cells to process and filter.
	 * @param BoundaryCells The cells acting as barriers.
	 * @param StartingCell The reference cell for proximity and direction.
	 * @return A set of filtered cells (`FBmrCells`) that meet the criteria. */
	static FBmrCells FilterCellsByBounds(const FBmrCells& ActiveCells, const FBmrCells& BoundaryCells, const FBmrCell& StartingCell);

	/** Returns true if the Starting Cell is in line of sight (in the direction of the Target Cell) within the given angle when comparing cells on the grid.
	 * Might be useful for AI to check does it see the bomb or player in the line of sight.
	 * @param StartingCell The reference starting cell (who is looking).
	 * @param TargetCell The target cell to check direction to (that can be seen).
	 * @param AllVisibleCells All cells that can be seen from the starting cell, the grid or just part of it.
	 * @param MaxAngleDegrees The maximum allowable angle (in degrees) for alignment, is recommended around 20 degrees. */
	static bool CanCellSeeTarget(const FBmrCell& StartingCell, const FBmrCell& TargetCell, const FBmrCells& AllVisibleCells, float MaxAngleDegrees = 40.f);

	/** Fisher-Yates shuffle algorithm to reorder the array elements. */
	static void RandShuffle(FBmrCellsArr& InOutArray);

	/** Extracts the top-left quarter of the grid. */
	static FBmrCellsArr GetTopLeftQuarterOnGrid(const FBmrCellsArr& InGrid, const FIntPoint& MapScale);

	/*********************************************************************************************
	 * Distance
	 ********************************************************************************************* */
public:
	/** Returns how many cells are between two cells, where each 1 unit means one cell. */
	template <typename T>
	static FORCEINLINE T Distance(const FBmrCell& C1, const FBmrCell& C2) { return FMath::Abs<T>((C1.Location - C2.Location).Size()) / CellSize; }

	/** Find the max distance between cells within specified set, where each 1 unit means one cell. */
	template <typename T>
	static T GetCellArrayMaxDistance(const FBmrCells& Cells);

	/*********************************************************************************************
	 * Math operators
	 ********************************************************************************************* */
public:
	/**
	 * Compares cells for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise
	 */
	FORCEINLINE bool operator==(const FBmrCell& Other) const { return this->Location == Other.Location; }
	FORCEINLINE bool operator!=(const FBmrCell& Other) const { return !(*this == Other); }

	/** Addition of cells. */
	FBmrCell& operator+=(const FBmrCell& Other);
	friend FORCEINLINE FBmrCell operator+(const FBmrCell& Lhs, const FBmrCell& Rhs) { return FBmrCell(Lhs.Location + Rhs.Location); }

	/** Subtracts a cell from another cell. */
	FBmrCell& operator-=(const FBmrCell& Other);
	friend FORCEINLINE FBmrCell operator-(const FBmrCell& Lhs, const FBmrCell& Rhs) { return FBmrCell(Lhs.Location - Rhs.Location); }

	/** Scales the cell, where FArg requires scalar type. */
	template <typename FArg UE_REQUIRES(std::is_arithmetic_v<FArg>)>
	FORCEINLINE FBmrCell operator*(FArg Scale) const { return FBmrCell(Location * Scale); }

	/** Creates a hash value from a FBmrCell.
	 * @param Vector the cell to create a hash value for
	 * @return The hash value from the components. */
	friend FORCEINLINE uint32 GetTypeHash(const FBmrCell& Vector) { return GetTypeHash(Vector.Location); }

	/*********************************************************************************************
	 * Conversion
	 ********************************************************************************************* */
public:
	/** Vector operator to return cell location. */
	FORCEINLINE operator FVector() const { return this->Location; }
	operator FVector_NetQuantize() const;

	/** Returns the cell as a string. */
	FString ToString() const { return FVector2D(Location).ToString(); }

	/** Returns the cell direction by its enum. */
	static const FBmrCell& GetCellDirection(EBmrCellDirection CellDirection);
	static EBmrCellDirection GetCellDirection(const FBmrCell& CellDirection);

	/** Puts specified cell in the cells set.*/
	static FORCEINLINE FBmrCells CellToCells(const FBmrCell& InCell) { return FBmrCells{InCell}; }
	FBmrCells ToCells() const { return CellToCells(*this); }

	/** Converts set of cells to array of vectors and vice versa. */
	static TArray<FVector> CellsToVectors(const FBmrCells& Cells);
	static FBmrCells VectorsToCells(const TArray<FVector>& Vectors);
};

// Find the max distance between cells within specified set, where each 1 unit means one cell
template <typename T>
T FBmrCell::GetCellArrayMaxDistance(const FBmrCells& Cells)
{
	T MaxDistance{};
	for (const FBmrCell& C1 : Cells)
	{
		for (const FBmrCell& C2 : Cells)
		{
			const T LengthIt = FBmrCell::Distance<T>(C1, C2);
			if (LengthIt > MaxDistance)
			{
				MaxDistance = LengthIt;
			}
		}
	}
	return MaxDistance;
}