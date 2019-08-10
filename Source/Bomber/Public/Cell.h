// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.generated.h"

/** The structure that contains the location vector of each element on the Level Map */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	/** The zero cell (0,0,0) */
	static const FCell ZeroCell;

	/** Holds the cell's FVector-coordinate. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector Location;

	/** Default constructor (zero initialization). */
	FCell()
		: Location(FVector::ZeroVector) {}

	/**
	 * Constructor
	 * Finds the nearest cell in the Grid Array for this level  actor
	 * 
	 * @param Actor Target to find cell location
	 * @return The cell that was found
	 * @bug #4 Length between two cells is not exactly equal 200
   	 * @todo Round to 200*(cos(fi)|sin(fi)
	 */
	explicit FCell(const AActor* Actor);

	/**
	* Constructor
	* Round another FVector into this cell
	*
	* @param Vector The other vector.

	*/
	explicit FORCEINLINE FCell(struct FVector Vector)
	{
		Location.X = FMath::RoundToFloat((Vector.X));
		Location.Y = FMath::RoundToFloat(Vector.Y);
		Location.Z = FMath::RoundToFloat(Vector.Z);
	}

	/**
	 * Compares points for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise
	 */
	FORCEINLINE bool operator==(const FCell& Other) const
	{
		return (this->Location == Other.Location);
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
