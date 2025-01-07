// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GeneratedMapSettings.generated.h"

/**
 * Contains settings for runtime generation of the level map. 
 */
USTRUCT(BlueprintType)
struct BOMBER_API FGeneratedMapSettings
{
	GENERATED_BODY()

	/** The chance of walls generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties, Units = "Percent", ClampMin = "0", ClampMax = "100"))
	int32 WallsChance = 75;

	/** The chance of boxes generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties, Units = "Percent", ClampMin = "0", ClampMax = "100"))
	int32 BoxesChance = 75;

	/** If true, the level position will be locked on the (0,0,0) location.
	 * Disable to allow the Generated Map to be moved around the scene. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
	bool LockOnZero = true;
};