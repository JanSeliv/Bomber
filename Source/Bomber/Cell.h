// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.generated.h"

/** The structure that contains the location vector of each element on the Level Map */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	/** Holds the cell's FVector-coordinate. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector Location = FVector::ZeroVector;

	/** Default constructor (zero initialization). */
	FCell() {}

	/**
	 * Constructor
	 * Find nearest location as cell on Grid Array
	 * 
	 * @param Actor Target to find cell location
	 * @return The cell that was found
	 */
	explicit FCell(const class AActor* Actor);

	/**
	* Constructor
	* Round another FVector into this cell
	*
	* @param Vector The other vector.
	* @return Reference to cell after copy.
	*/
	explicit FORCEINLINE FCell(struct FVector Vector)
	{
		this->Location.X = FMath::RoundToFloat(Vector.X);
		this->Location.Y = FMath::RoundToFloat(Vector.Y);
		this->Location.Z = FMath::RoundToFloat(Vector.Z);
	}

	/**
	 * Compares points for equality.
	 *
	 * @param Other The other cell being compared.
	 * @return true if the points are equal, false otherwise..
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
