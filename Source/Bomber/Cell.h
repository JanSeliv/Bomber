// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.generated.h"

USTRUCT(BlueprintType, meta = (HasNativeMake = "Bomber.SingletonLibrary.MakeCell"))
struct FCell
{
	GENERATED_BODY()

	FCell(){};

	FCell(const AActor* actor);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector location;

	bool operator==(const FCell& other) const
	{
		return (this->location == other.location);
	}
	// Hash Function
	friend FORCEINLINE uint32 GetTypeHash(const FCell& other)
	{
		return GetTypeHash(other.location);
	}
};