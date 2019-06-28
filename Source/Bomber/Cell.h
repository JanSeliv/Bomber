// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.generated.h"

USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	/** Sets default values */
	FCell(){};

	/** @defgroup cell_functions Group with cell functions
	 * 
	 * @param Actor 
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
