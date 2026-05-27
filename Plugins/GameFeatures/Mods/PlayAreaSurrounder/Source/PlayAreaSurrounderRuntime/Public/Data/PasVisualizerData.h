// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "Math/Color.h" // FLinearColor

#include "PasVisualizerData.generated.h"

/**
 * Contains data to set up visualization helper of dangerous cell where surrounder is going to spawn next wall.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FPasVisualizerData
{
	GENERATED_BODY()

	/** Amount of light blinks per cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Play Area Surrounder]", meta = (ClampMin = "0"))
	int32 FlickersNumber = 2;

	/** Color of visualizer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Play Area Surrounder]")
	FLinearColor Color = FLinearColor::Red;

	/** Height percentage from floor, where 100% is size of cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Play Area Surrounder]", meta = (ClampMin = "0", Units = "Percent"))
	int32 HeightPercent = 200;

	/** Wide percentage of visualizer, where 100% is size of cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Play Area Surrounder]", meta = (ClampMin = "0", Units = "Percent"))
	int32 WidePercent = 90;

	/** Brightness of visualizer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Play Area Surrounder]", meta = (ClampMin = "0.0", Units = "Candela"))
	float Intensity = 500.f;
};
