// Copyright (c) Yevhenii Selivanov

#pragma once

// PAS
#include "PasTurnType.h"

#include "PasCellDataOnSide.generated.h"

/**
 * Contains transient Surrounder data about current position on play area.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FPasCellDataOnSide
{
	GENERATED_BODY()

	/** Empty structure data. */
	static const FPasCellDataOnSide EmptyData;

	/** Current side where Surrounder locates. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	EPasTurnType TurnType = EPasTurnType::None;

	/** Number of horizontal right sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Hr = 0;

	/** Number of vertical down passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Vd = 0;

	/** Number of horizontal left sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Hl = 0;

	/** Number of vertical up sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Vu = 0;

	/** Current row index. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Row = 0;

	/** Current column index. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "[Play Area Surrounder]")
	int32 Column = 0;
};
