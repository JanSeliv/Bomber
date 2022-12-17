// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Globals/LevelActorDataAsset.h"
//---
#include "WallDataAsset.generated.h"

/**
* Describes common data for all walls.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UWallDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UWallDataAsset();

	/** Returns the wall data asset. */
	static const UWallDataAsset& Get();
};