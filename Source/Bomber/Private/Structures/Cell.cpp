// Copyright 2021 Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
//---

// The zero cell
const FCell FCell::ZeroCell = FCell();

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(FVector Vector)
{
	Location.X = FMath::RoundToFloat((Vector.X));
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(AGeneratedMap::Get().GetActorLocation().Z);
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(const float& AxisZ) const
{
	if (!ensureAlwaysMsgf(AxisZ != abs(0.f), TEXT("The axis is zero")))
	{
		return *this;
	}

	const AGeneratedMap& LevelMap = AGeneratedMap::Get();
	const FVector Dimensions = this->Location - LevelMap.GetActorLocation();
	const FVector RotatedVector = Dimensions.RotateAngleAxis(LevelMap.GetActorRotation().Yaw, FVector(0, 0, AxisZ));
	return FCell(this->Location + RotatedVector - Dimensions);
}
