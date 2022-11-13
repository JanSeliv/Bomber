// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
#include "PlayAreaSurrounderTypes.h"
#include "PlayAreaSurrounderData.generated.h"

/**
 * Contains general settings of 'PlayAreaSurrounder' game feature.
 */
UCLASS(Blueprintable, BlueprintType)
class PLAYAREASURROUNDERRUNTIME_API UPlayAreaSurrounderData final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The time in seconds before Surrounder starts spawning the walls. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", Units = "Seconds"))
	float WaitBeforeSurroundTime = 3.f; //[D]

	/** The time in seconds takes Surrounder to spawn next wall. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", Units = "Seconds"))
	float WallStepTime = 1.5f; //[D]

	/** The time in seconds that decreases overall waiting for Surrounder start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", Units = "Seconds"))
	float WaitOnDeathShortenerTime = 1.f; //[D]

	/** When true, the Surrounder starts spawning on random side. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRandomlySelectTurnType = true; //[D]

	/** The amount of cells Surrounder should notify bots about danger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 UnsafeCellsNumInFront = 3; //[D]

	/** Contains data to set up visualization helper of a dangerous cell where surrounder is going to spawn the next wall. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visualizer Data")
	FVisualizerData VisualizerData = FVisualizerData::DefaultData; //[D]
};