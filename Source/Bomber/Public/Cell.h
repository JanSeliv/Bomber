// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Cell.generated.h"

/** Typedef to allow for some nicer looking sets of cells */
typedef TSet<struct FCell> FCells;

/**
 * The structure that contains a location of an one cell on a grid of the Level Map.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	/** The zero cell (0,0,0) */
	static const FCell ZeroCell;

	/** The length of the one cell */
	static const float CellSize;

	/** Always holds the free cell's FVector-coordinate.
	 * If it is not empty or not found, holds the last succeeded due to copy operator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	FVector Location = FVector::ZeroVector;  //[AW]

	/** Marks when the cell is contained in the grid and free from other level actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	bool bWasFound = false;  //[B]

	/** Default constructor (zero initialization). */
	FCell() {}

	/**
	 * The main constructor.
	 * Finds the nearest free cell in the Grid Array for the specified Map Component's owner. 
	 * 
	 * @param MapComponent Target to find the cell.
	 */
	explicit FCell(const class UMapComponent* MapComponent);

	/**
	* Initial constructor for cells filling into the array.
	* Round another FVector into this cell.
	*
	* @param Vector The other vector.
	*/
	explicit FCell(struct FVector Vector);

	/** Rotates around the center of the Level Map to the same yaw degree.
	 * 
	 * @param AxisZ The Z param of the axis to rotate around
	 * @return Rotated to the Level Map cell, the same cell otherwise
	 */
	FCell RotateAngleAxis(const float& AxisZ) const;

	/* Set the cell to zero value. */
	FORCEINLINE void SetToZero()
	{
		Location = ZeroCell.Location;
	}

	/**
	* Copy another non-zero cell into this one.
	*
	* @param Other The other cell.
	* @return Reference to cell after copy.
	*/
	FCell& operator=(const FCell& Other);

	/**
	 * Compares cells for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise
	 */
	FORCEINLINE bool operator==(const FCell& Other) const
	{
		return this->Location == Other.Location;
	}

	/**
	* Creates a hash value from a FCell. 
	*
	* @param Vector the cell to create a hash value for
	* @return The hash value from the components
	*/
	friend FORCEINLINE uint32 GetTypeHash(const FCell& Vector)
	{
		return GetTypeHash(Vector.Location);
	}
};
