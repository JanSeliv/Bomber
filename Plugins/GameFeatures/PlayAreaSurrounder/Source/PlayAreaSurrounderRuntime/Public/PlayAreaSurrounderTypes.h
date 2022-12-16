// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Math/Color.h"
#include "PlayAreaSurrounderTypes.generated.h"

/**
 * Specifies the side of the play area surrounder.
 * ══════1═════════╗
 * ╔═════1═════╗   ║
 * ║   ╔═1═╗   ║   ║
 * ║   ║   2   2   2
 * 4   4   ╨   ║   ║
 * ║   ╚═══3═══╝   ║
 * ╚═══════3═══════╝
 * 1: HorizontalRight
 * 2: VerticalDown
 * 3: HorizontalLeft
 * 4: VerticalUp
 */
UENUM(BlueprintType)
enum class ETurnType : uint8
{
	None,
	HorizontalRight,
	VerticalDown,
	HorizontalLeft,
	VerticalUp
};

/**
 * Contains data to set up visualization helper of a dangerous cell where surrounder is going to spawn the next wall.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FVisualizerData
{
	GENERATED_BODY()

	/** Visualization data init by default.*/
	static const FVisualizerData DefaultData;

	/** Amount of light blinks per cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 FlickersNumber = 2;

	/** The color of visualizer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::Red;

	/** Height percentage from floor, where 100% is size of cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "Percent"))
	int32 HeightPercent = 200.f;

	/** Wide percentage of visualizer, where 100% is size of cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "Percent"))
	int32 WidePercent = 90.f;

	/** The brightness of visualizer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", Units = "Candela"))
	float Intensity = 500.f;
};

/**
 * Contains transient Surrounder data about current position on the play area.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FCellDataOnSide
{
	GENERATED_BODY()

	/** Empty structure data. */
	static const FCellDataOnSide EmptyData;

	/** Current side where Surrounder locates. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	ETurnType TurnType = ETurnType::None;

	/** Number of horizontal right sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Hr = 0;

	/** Number of vertical down passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Vd = 0;

	/** Number of horizontal left sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Hl = 0;

	/** Number of vertical up sides passed by Surrounder. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Vu = 0;

	/** Current row index. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Row = 0;

	/** Current column index. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	int32 Column = 0;
};

/**
 * Contains transient Surrounder output data of found next cell.
 */
USTRUCT(BlueprintType)
struct PLAYAREASURROUNDERRUNTIME_API FNextCellOutput
{
	GENERATED_BODY()

	/** Empty structure data. */
	static const FNextCellOutput EmptyData;

	/** Transient data about current position of the Surrounder on the play area. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	FCellDataOnSide CellDataOnSurrounderSide = FCellDataOnSide::EmptyData;

	/** Is true if Surrounder was finished. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	bool bIsOutsideBoundaries = false;

	/** Is true if next cell has last position on current side. */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++")
	bool bIsLastCellOnSide = false;
};
