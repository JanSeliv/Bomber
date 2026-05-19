// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GfpmUtils.generated.h"

struct FGfpmStateChange;

enum class EGameFeatureTargetState : uint8;

/**
 * Function library with Game Feature Plugin (GFP) helpers.
 */
UCLASS(DisplayName = "Game Feature Plugins Manager Utils")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if specified game feature plugin is currently active.
	 * @param GameFeatureName The name of the game feature plugin to check. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static bool IsGameFeaturePluginActive(FName GameFeatureName);

	/** Returns the built-in initial target state for a game feature.
	 * @param GameFeatureName The name of the game feature plugin. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static EGameFeatureTargetState GetBuiltInInitialFeatureState(FName GameFeatureName);

	/** Enables or disable all game features. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	static void SetGameFeaturePluginsActive(bool bEnable, const TArray<FName>& GameFeatures);

	/** Changes target state for game features, batching all requests by state.
	 * @param Changes Array of game features and their desired target states. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	static void ChangeGameFeatureTargetState(const TArray<FGfpmStateChange>& Changes);

	/** Resets game features to their configured built-in auto state.
	 * @param GameFeatures Array of game feature names to reset. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	static void RestoreGameFeatureTargetState(const TArray<FName>& GameFeatures);

	/** Returns the content module name from the specified asset package, e.g. "/GameFeatureModule/" from a content asset. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static FString GetModuleNameByAsset(const UObject* Asset);

	/** Returns the module name from any object by resolving its class package.
	 * For C++ objects, extracts from /Script/ package (e.g. "GameFeatureModuleRuntime").
	 * For Blueprint objects, extracts content root from class package (e.g. "GameFeatureModule"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static FString GetModuleNameByObject(const UObject* Object);

	/** Returns true if the given object belongs to the same game feature plugin as the specified GameFeatureData.
	 * Compares the object's module name against the plugin content root. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static bool IsInGameFeatureModule(const UObject* Object, const class UGameFeatureData* GameFeatureData);

	/** Returns true if the given object belongs to any registered game feature plugin.
	 * For C++ objects, checks module name against registered features; for Blueprint objects, checks class content root. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static bool IsInAnyGameFeatureModule(const UObject* Object);

	/** Returns names of all registered game feature plugins
	 * Is mostly used by `meta = (GetOptions = "GameFeaturePluginsManager.GfpmUtils.GetAllRegisteredGameFeaturePlugins"))` */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]")
	static TArray<FString> GetAllRegisteredGameFeaturePlugins();

	/** Unloads the specified asset from memory. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	static void UnloadAsset(UObject* AssetToUnload, bool bUnloadReferences = false);

	/** Unloads every asset in the given array from memory.
	 * Accepts any array of UObject-derived pointers; loops the array and unloads each element. */
	template <typename T>
	static void UnloadAssets(const TArray<T*>& AssetsToUnload, bool bUnloadReferences = false);
};

template <typename T>
void UGfpmUtils::UnloadAssets(const TArray<T*>& AssetsToUnload, bool bUnloadReferences)
{
	for (T* AssetIt : AssetsToUnload)
	{
		UnloadAsset(AssetIt, bUnloadReferences);
	}
}