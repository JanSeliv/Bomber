// Copyright 2021 Yevhenii Selivanov.

#include "Structures/Cell.h"
//---
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
//---

// The zero cell
const FCell FCell::ZeroCell = FCell();

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(const FVector& Vector)
{
	Location.X = FMath::RoundToFloat(Vector.X);
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(AGeneratedMap::Get().GetCachedTransform().GetLocation().Z);
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(float AxisZ) const
{
	const FTransform& LevelTransform = AGeneratedMap::Get().GetCachedTransform();
	const FVector Dimensions = Location - LevelTransform.GetLocation();
	const float AngleDeg = LevelTransform.GetRotation().Rotator().Yaw;
	const FVector Axis(0, 0, AxisZ);
	const FVector RotatedVector = Dimensions.RotateAngleAxis(AngleDeg, Axis);
	return FCell(Location + RotatedVector - Dimensions);
}
