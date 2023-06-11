// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "FootTrailsDataAsset.generated.h"

class UDataTable;

/**
 *  Contains all the assets and tweaks of Foot Trails game feature.
 */
UCLASS(BlueprintType, Blueprintable)
class FOOTTRAILSGENERATORRUNTIME_API UFootTrailsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the data asset that contains all foot trails archetypes.
	 * @see UFootTrailsGeneratorComponent::FootTrailsDataTableInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE UDataTable* GetFootTrailsDataTable() const { return FootTrailsDataTableInternal; }

protected:
	/** The data asset that contains all foot trails archetypes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Foot Trails Data Table", ShowOnlyInnerProperties))
	TObjectPtr<UDataTable> FootTrailsDataTableInternal = nullptr;
};
