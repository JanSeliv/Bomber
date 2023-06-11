// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "FootTrailsUtils.generated.h"

/**
 * Blueprint utilities for Foot Trails Generator plugin.
 */
UCLASS()
class FOOTTRAILSGENERATORRUNTIME_API UFootTrailsUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the Foot Trails Generator component that is responsible for generating foot trails. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UFootTrailsGeneratorComponent* GetFootTrailsGeneratorComponent();

	/** Returns the data asset that contains all the assets and tweaks of Foot Trails game feature. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static const class UFootTrailsDataAsset* GetFootTrailsDataAsset();
};
