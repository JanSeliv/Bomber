// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// PAS
#include "Data/PasVisualizerData.h"

#include "PasDataAsset.generated.h"

/**
 * Contains general settings of 'PlayAreaSurrounder' game feature.
 */
UCLASS(Blueprintable, BlueprintType)
class PLAYAREASURROUNDERRUNTIME_API UPasDataAsset final : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns PlayAreaSurrounder data asset or crash when cannot be obtained. */
	static const UPasDataAsset& Get();

	/** Time in seconds before Surrounder starts spawning walls. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]", meta = (ClampMin = "0.0", Units = "Seconds"))
	float WaitBeforeSurroundTime = 3.f;

	/** The time in seconds takes Surrounder to spawn next wall. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]", meta = (ClampMin = "0.0", Units = "Seconds"))
	float WallStepTime = 1.5f;

	/** The multiplier to increase Surrounder speed after passing each side.
	 * WallStepTime / ((WallStepTimeMultiplierForEachSide * PassedSidesNum) + 1)
	 * if WallStepTime = 1.5 and WallStepTimeMultiplierForEachSide = 0.25, then:
	 * Passed 0 sides (game started): 1.5 / ((0.25 * 0) + 1) = 1.2 between walls.
	 * Passed 1 side: 1.5 / ((0.25 * 1) + 1) = 1.125 between walls.
	 * Passed 2 sides: 1.5 / ((0.25 * 2) + 1) = 1 between walls.
	 * Passed 3 sides: 1.5 / ((0.25 * 3) + 1) = 0.85 between walls.
	 * Passed 10 sides: 1.5 / ((0.25 * 10) + 1) = 0.42 between walls. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]", meta = (ClampMin = "0.0"))
	float WallStepTimeMultiplierForEachSide = 0.25f;

	/** The time in seconds that decreases overall waiting for Surrounder start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]", meta = (ClampMin = "0.0", Units = "Seconds"))
	float WaitOnDeathShortenerTime = 1.f;

	/** When true, the Surrounder starts spawning on random side. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]")
	bool bRandomlySelectTurnType = true;

	/** The amount of cells Surrounder should notify bots about danger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]", meta = (ClampMin = "0"))
	int32 UnsafeCellsNumInFront = 3;

	/** Contains data to set up visualization helper of a dangerous cell where surrounder is going to spawn the next wall. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Play Area Surrounder]|Visualizer Data")
	FPasVisualizerData VisualizerData;
};
