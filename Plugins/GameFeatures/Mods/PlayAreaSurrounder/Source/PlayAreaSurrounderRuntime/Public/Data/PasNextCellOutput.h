// Copyright (c) Yevhenii Selivanov

#pragma once

// PAS
#include "PasCellDataOnSide.h"

#include "PasNextCellOutput.generated.h"

/**
 * Contains transient Surrounder output data of found next cell.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FPasNextCellOutput
{
	GENERATED_BODY()

	/** Transient data about current position of Surrounder on play area. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	FPasCellDataOnSide CellDataOnSurrounderSide = FPasCellDataOnSide::EmptyData;

	/** Is true if Surrounder was finished. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	bool bIsOutsideBoundaries = false;

	/** Is true if next cell has last position on current side. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	bool bIsLastCellOnSide = false;
};
