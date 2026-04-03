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

	/** Returns the content module name from the specified asset package, e.g. "/GameFeatureModule/" from a content asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FString GetModuleNameByAsset(const UObject* Asset);

	/** Returns the module name from any object by resolving its class package.
	 * For C++ objects, extracts from /Script/ package (e.g. "GameFeatureModuleRuntime").
	 * For Blueprint objects, extracts content root from class package (e.g. "GameFeatureModule"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FString GetModuleNameByObject(const UObject* Object);

	/** Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData.
	 * Compares the object's module name against the plugin content root. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsInGameFeatureModule(const UObject* Object, const class UGameFeatureData* GameFeatureData);

	/** Returns true if the given object belongs to any registered game feature plugin.
	 * For C++ objects, checks module name against registered features; for Blueprint objects, checks class content root. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static bool IsInAnyGameFeatureModule(const UObject* Object);

	/** Returns names of all registered Modular Game Feature plugins
	 * Is mostly used by `meta = (GetOptions = "MyUtils.ModularGameFeaturePluginUtils.GetAllRegisteredModularGameFeatures"))` */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static TArray<FString> GetAllRegisteredModularGameFeatures();

	/** Unloads the specified asset from memory. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences = false);
};