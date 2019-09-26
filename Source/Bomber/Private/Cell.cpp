// Copyright 2019 Yevhenii Selivanov.

#include "Cell.h"

#include "GameFramework/Actor.h"

#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"

// The zero cell
const FCell FCell::ZeroCell = FCell();

// The length of the one cell
const float FCell::CellSize = 200.0F;

// Initial constructor for cells filling into the array. Round another FVector into this cell.
FCell::FCell(FVector Vector)
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	bWasFound = IsValid(LevelMap);
	Location.X = FMath::RoundToFloat((Vector.X));
	Location.Y = FMath::RoundToFloat(Vector.Y);
	Location.Z = FMath::RoundToFloat(bWasFound ? LevelMap->GetActorLocation().Z : Vector.Z);
}

// Rotates around the center of the Level Map to the same yaw degree
FCell FCell::RotateAngleAxis(const float& AxisZ) const
{
	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IsValid(LevelMap) == false  //
		|| !ensureAlwaysMsgf(AxisZ != abs(0.f), TEXT("The axis is zero")))
	{
		return *this;
	}

	const FVector Dimensions = this->Location - LevelMap->GetActorLocation();
	const FVector RotatedVector = Dimensions.RotateAngleAxis(LevelMap->GetActorRotation().Yaw, FVector(0, 0, AxisZ));
	return FCell(this->Location + RotatedVector - Dimensions);
}
