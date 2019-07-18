// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.generated.h"

/** The structure that contains the location vector of each element on the Level Map */
USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	/** Sets default values */
	FCell();

	/** @defgroup cell_functions Group with cell functions
	 * Find nearest location as cell on Grid Array
	 * @param Actor Target to find cell location
	 * @return The cell that was found
	 */
	explicit FCell(const AActor* Actor);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector Location;

	bool operator==(const FCell& Other) const
	{
		return (this->Location == Other.Location);
	}

	// Hash Function
	friend FORCEINLINE uint32 GetTypeHash(const FCell& Other)
	{
		return GetTypeHash(Other.Location);
	}
};
