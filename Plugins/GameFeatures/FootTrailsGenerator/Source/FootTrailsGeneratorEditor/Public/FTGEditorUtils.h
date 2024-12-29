// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FTGEditorUtils.generated.h"

/**
 * Helpers for editor functions of the Foot Trails Generator.
 */
UCLASS()
class FOOTTRAILSGENERATOREDITOR_API UFTGEditorUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Tries to obtain the blueprint class of the Foot Trails Generator component from MGF data asset, where it's expected to be set. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DevelopmentOnly))
	static TSubclassOf<class UFTGComponent> GetFootTrailsComponentClass();
};
