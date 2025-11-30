// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"

#include "FTGDataAsset.generated.h"

class UDataTable;

/**
 *  Contains all the assets and tweaks of Foot Trails game feature.
 */
UCLASS(BlueprintType, Blueprintable)
class FOOTTRAILSGENERATORRUNTIME_API UFTGDataAsset : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the data asset that contains all foot trails archetypes. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[FootTrailsGenerator]")
	FORCEINLINE UDataTable* GetFootTrailsDataTable() const { return FootTrailsDataTable; }

protected:
	/** The data asset that contains all foot trails archetypes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected))
	TObjectPtr<UDataTable> FootTrailsDataTable = nullptr;
};
