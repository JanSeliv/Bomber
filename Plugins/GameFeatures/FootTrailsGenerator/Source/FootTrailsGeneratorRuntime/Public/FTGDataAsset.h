// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

#include "FTGDataAsset.generated.h"

class UDataTable;

/**
 *  Contains all the assets and tweaks of Foot Trails game feature.
 */
UCLASS(BlueprintType, Blueprintable)
class FOOTTRAILSGENERATORRUNTIME_API UFTGDataAsset : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns this data asset, is checked and will crash if can't be obtained */
	static const UFTGDataAsset& Get();

	/** Returns the data asset that contains all foot trails archetypes. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[FootTrailsGenerator]")
	FORCEINLINE UDataTable* GetFootTrailsDataTable() const { return FootTrailsDataTable; }

protected:
	/** The data asset that contains all foot trails archetypes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TObjectPtr<UDataTable> FootTrailsDataTable = nullptr;
};
