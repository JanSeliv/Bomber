// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Cell.generated.h"

/** Typedef to allow for some nicer looking sets of cells. */
typedef TSet<struct FCell> FCells;

/**
 * Represents one of direction of a cell.
 */
UENUM(BlueprintType)
enum class ECellDirection : uint8
{
	None,
	Forward,
	Backward,
	Right,
	Left
};

/**
 * The structure that contains a location of an one cell on a grid of the Level Map.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.CellsUtilsLibrary.MakeCell", HasNativeBreak = "Bomber.CellsUtilsLibrary.BreakCell"))
struct BOMBER_API FCell
{
	GENERATED_BODY()

	static const FCell ZeroCell;
	static const FCell ForwardCell;
	static const FCell BackwardCell;
	static const FCell RightCell;
	static const FCell LeftCell;

	/** The length of the one cell */
	static constexpr float CellSize = 200.f;

	/** Always holds the free cell's FVector-coordinate.
	 * If it is not empty or not found, holds the last succeeded due to copy operator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FVector Location = FVector::DownVector; //[AW]

	/** Default constructor. */
	FCell() = default;

	/**
	* Initial constructor for cells filling into the array.
	* Round another FVector into this cell.
	*
	* @param Vector The other vector.
	*/
	FCell(const FVector& Vector);

	/** Equal operator for vectors to directly copy its value to the cell. */
	FCell& operator=(const FVector& Vector);

	/** Rotates around the center of the Level Map to the same yaw degree.
	 *
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell, the same cell otherwise
	 */
	FCell RotateAngleAxis(float AxisZ) const;

	/** Comparing with uninitialized Zero Cell. */
	FORCEINLINE bool IsZeroCell() const { return *this == ZeroCell; }

	/** Check is valid this cell. */
	FORCEINLINE bool IsValid() const { return *this != ZeroCell; }

	/** Returns how many cells are between two cells. */
	template <typename T>
	static FORCEINLINE T Distance(const FCell& C1, const FCell& C2) { return FMath::Abs<T>((C1.Location - C2.Location).Size()) / CellSize; }

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
	static FCell GetCellArrayAverage(const TSet<FCell>& Cells);

	/** Returns the cell direction by its enum. */
	static const FCell& GetCellDirection(ECellDirection CellDirection);

	/**
	* Creates a hash value from a FCell.
	*
	* @param Vector the cell to create a hash value for
	* @return The hash value from the components
	*/
	friend FORCEINLINE uint32 GetTypeHash(const FCell& Vector) { return GetTypeHash(Vector.Location); }
};
