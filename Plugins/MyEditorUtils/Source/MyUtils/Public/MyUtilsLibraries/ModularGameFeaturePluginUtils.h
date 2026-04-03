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

	/** Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData.
	 * For content objects (Blueprints, Data Assets), compares package root paths directly.
	 * For C++ runtime objects (subsystems, components), resolves the class module name against the plugin content root. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsInGameFeatureModule(const UObject* Object, const class UGameFeatureData* GameFeatureData);

	/** Returns names of all registered Modular Game Feature plugins
	 * Is mostly used by `meta = (GetOptions = "MyUtils.ModularGameFeaturePluginUtils.GetAllRegisteredModularGameFeatures"))` */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static TArray<FString> GetAllRegisteredModularGameFeatures();

	/** Unloads the specified asset from memory. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences = false);
};