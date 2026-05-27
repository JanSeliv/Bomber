// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

struct FPasCellDataOnSide;

enum class EPasTurnType : uint8;

#include "PasBlueprintFunctionLibrary.generated.h"

/**
 * Hosts cell/side math, data-asset accessor, wall/flicker time formulas.
 * Server-and-client pure helpers shared by surrounder + prediction components.
 */
UCLASS()
class PLAYAREASURROUNDERRUNTIME_API UPasBlueprintFunctionLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns loaded PlayAreaSurrounder data asset, null if not yet loaded. */
	UFUNCTION(BlueprintPure, Category = "[Play Area Surrounder]")
	static const class UPasDataAsset* GetPlayAreaSurrounderData();

	/** Computes next cell on surrounder's current side.
	 * @TODO JanSeliv Vqol05dU - Improve algorithm efficiency. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static struct FPasNextCellOutput GetNextCell(const FPasCellDataOnSide& InCellData);

	/** Returns starting cell of requested side, or invalid cell when none exists. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static struct FBmrCell GetFirstCellOnSide(EPasTurnType TurnType);

	/**
	 * Returns next `Num` predicted cells from `InCellData`, stopping at boundaries.
	 * @param InCellData Current cell + side state to walk forward from.
	 * @param Num How many forward cells to emit, loop stops early at boundary.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static TArray<FPasCellDataOnSide> GetNextCells(const FPasCellDataOnSide& InCellData, int32 Num);

	/** Flattens array of `FPasCellDataOnSide` into underlying cell set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static TSet<struct FBmrCell> GetCellsFromData(const TArray<FPasCellDataOnSide>& CellsData);

	/** Returns active per-step delay between wall spawns, accelerating after each completed side. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static float CalcWallStepTime(int32 PassedSidesNum);

	/** Returns active per-flicker delay derived from per-step delay split by flicker count. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Play Area Surrounder]")
	static float CalcFlickerTime(int32 PassedSidesNum);
};
