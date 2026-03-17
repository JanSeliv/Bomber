// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ModularGameFeaturePluginUtils.generated.h"

/**
 * Function library with Modular Game Feature (MGF) plugin helpers.
 */
UCLASS()
class MYUTILS_API UModularGameFeaturePluginUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if specified Modular Game Feature plugin is currently active.
	 * @param GameFeatureName The name of the game feature plugin to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsModularGameFeatureActive(FName GameFeatureName);

	/** Enables or disable all game features. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void SetModularGameFeaturesActive(bool bEnable, const TArray<FName>& GameFeatures);

	/** Returns the module name from the specified asset, if it is part of a game feature. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FString GetModuleNameFromAsset(const UObject* Asset);

	/** Unloads the specified asset from memory. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences = false);
};