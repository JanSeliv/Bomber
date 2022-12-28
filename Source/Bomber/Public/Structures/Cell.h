// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Cell.generated.h"

/** Typedef to allow for some nicer looking sets of cells. */
typedef TSet<struct FCell> FCells;

/**
 * Represents one of direction of a cell.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECellDirection : uint8
{
	None = 0,
	Forward = 1 << 0,
	Backward = 1 << 1,
	Right = 1 << 2,
	Left = 1 << 3,
	All = Forward | Backward | Right | Left
};

ENUM_CLASS_FLAGS(ECellDirection);
using ECD = ECellDirection;

/**
 * The structure that contains a location of an one cell on a grid of the Level Map.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Bomber.CellsUtilsLibrary.MakeCell", HasNativeBreak = "/Script/Bomber.CellsUtilsLibrary.BreakCell"))
struct BOMBER_API FCell
{
	GENERATED_BODY()

	static const FCell InvalidCell;
	static const FCell ForwardCell;
	static const FCell BackwardCell;
	static const FCell RightCell;
	static const FCell LeftCell;
	static const FCells EmptyCells;

	/** The length of the one cell */
	static constexpr float CellSize = 200.f;

	/** Always holds the free cell's FVector-coordinate.
	 * If it is not empty or not found, holds the last succeeded due to copy operator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FVector Location = FVector::DownVector;

	/** Default constructor. */
	FCell() = default;

	/** Vector to cell constructor. */
	FCell(const FVector& Vector);

	/** Floats to cell constructor. */
	explicit FCell(float X, float Y, float Z);

	/** Doubles to cell constructor. */
	explicit FCell(double X, double Y, double Z);

	/** Equal operator for vectors to directly copy its value to the cell. */
	FCell& operator=(const FVector& Vector);

	/** Returns Cell's X component. */
	float X() const { return Location.X; }

	/** Returns Cell's Y component. */
	float Y() const { return Location.Y; }

	/** Returns Cell's Z component. */
	float Z() const { return Location.Z; }

	/** Rotates around the center of the Level Map to the same yaw degree.
	 *
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell, the same cell otherwise
	 */
	FCell RotateAngleAxis(float AxisZ) const;

	/** Comparing with uninitialized Invalid Cell. */
	FORCEINLINE bool IsInvalidCell() const { return *this == InvalidCell; }

	/** Check is valid this cell. */
	FORCEINLINE bool IsValid() const { return *this != InvalidCell; }

	/** Returns how many cells are between two cells, where each 1 unit means one cell. */
	template <typename T>
	static FORCEINLINE T Distance(const FCell& C1, const FCell& C2) { return FMath::Abs<T>((C1.Location - C2.Location).Size()) / CellSize; }

	/** Find the max distance between cells within specified set, where each 1 unit means one cell. */
	template <typename T>
	static float GetCellArrayMaxDistance(const FCells& Cells);

	/**
	 * Compares cells for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise
	 */
	FORCEINLINE bool operator==(const FCell& Other) const { return this->Location == Other.Location; }
	FORCEINLINE bool operator!=(const FCell& Other) const { return !(*this == Other); }

	/** Addition of cells. */
	FCell& operator+=(const FCell& Other);
	friend FORCEINLINE FCell operator+(const FCell& Lhs, const FCell& Rhs) { return FCell(Lhs.Location + Rhs.Location); }

	/** Subtracts a cell from another cell. */
	FCell& operator-=(const FCell& Other);
	friend FORCEINLINE FCell operator-(const FCell& Lhs, const FCell& Rhs) { return FCell(Lhs.Location - Rhs.Location); }

	/** Vector operator to return cell location. */
	FORCEINLINE operator FVector() const { return this->Location; }

	/** Find the average of an set of cells. */
	static FCell GetCellArrayAverage(const FCells& Cells);

	/** Returns the cell direction by its enum. */
	static const FCell& GetCellDirection(ECellDirection CellDirection);
	static ECellDirection GetCellDirection(const FCell& CellDirection);

	/** Puts specified cell in the cells set.*/
	static FORCEINLINE FCells CellToCells(const FCell& InCell) { return FCells{InCell}; }
	FCells ToCells() const { return CellToCells(*this); }

	/** Extracts first cell from specified cells set.*/
	static FORCEINLINE FCell GetFirstCellInSet(const FCells& InCells) { return !InCells.IsEmpty() ? InCells.Array()[0] : InvalidCell; }

	/**
	* Creates a hash value from a FCell.
	*
	* @param Vector the cell to create a hash value for
	* @return The hash value from the components
	*/
	friend FORCEINLINE uint32 GetTypeHash(const FCell& Vector) { return GetTypeHash(Vector.Location); }
};

// Find the max distance between cells within specified set, where each 1 unit means one cell
template <typename T>
float FCell::GetCellArrayMaxDistance(const FCells& Cells)
{
	T MaxDistance{};
	for (const FCell& C1 : Cells)
	{
		for (const FCell& C2 : Cells)
		{
			const T LengthIt = FCell::Distance<T>(C1, C2);
			if (LengthIt > MaxDistance)
			{
				MaxDistance = LengthIt;
			}
		}
	}
	return MaxDistance;
}
