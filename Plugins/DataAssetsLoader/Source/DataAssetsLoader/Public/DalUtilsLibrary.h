// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "DalUtilsLibrary.generated.h"

/**
 * Blueprint utility class for Data Assets Loader.
 * Provides helper functions for querying data assets from the asset registry and Data Registries.
 */
UCLASS()
class DATAASSETSLOADER_API UDalUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data Assets
	 ********************************************************************************************* */
public:
	/** Returns the first found asset data for a UDalPrimaryDataAsset descendant matching a single registry tag.
	 * @param AssetTag The asset registry tag name to filter by
	 * @param AssetValue The expected tag value
	 * @return Found asset data, or empty FAssetData if not found */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static struct FAssetData GetAssetByRegistryTag(FName AssetTag, const FString& AssetValue);

	/** The same as GetAssetByRegistryTag, but accepts multiple tags and values, and returns all matching assets. */
	static void GetAssetsByRegistryTags(TArray<FAssetData>& OutAssetsData, const TMultiMap<FName, TOptional<FString>>& TagsAndValues);

	/** Returns primary asset IDs for all UDalPrimaryDataAsset descendants discovered under the game feature plugin root path.
	 * @param OutAssetIds Returns primary asset IDs found under the game feature path
	 * @param GameFeatureData The game feature data to extract the plugin root path from */
	UFUNCTION(BlueprintCallable, Category = "[DataAssetsLoader]")
	static void GetAssetsInGameFeaturePlugin(TArray<FPrimaryAssetId>& OutAssetIds, const class UGameFeatureData* GameFeatureData);

	/*********************************************************************************************
	 * Data Registry
	 ********************************************************************************************* */
public:
	/** Returns all cached rows from Data Registry as a typed array matching InRowStruct, used internally by UK2Node_GetAllRegistryRows */
	UFUNCTION(BlueprintPure, CustomThunk, Category = "[DataAssetsLoader]", meta = (CustomStructureParam = "OutRows", BlueprintInternalUseOnly = "true"))
	static void K2_GetAllRegistryRows(const UScriptStruct* InRowStruct, TArray<int32>& OutRows);
	static void K2_GetAllRegistryRowsGeneric(const UScriptStruct* InRowStruct, const class FArrayProperty* ArrayProp, void* OutArrayPtr);
	DECLARE_FUNCTION(execK2_GetAllRegistryRows);

	/** Finds the Data Registry whose ItemStruct matches InStruct, cached for fast repeated access */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static class UDataRegistry* GetRegistryForStruct(const UScriptStruct* InStruct);

	/** Returns overall number of cached rows for the given row struct type */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static int32 GetRegistryRowsNum(const UScriptStruct* InStruct);

	/** Returns the row name at specified index for the given struct type, or NAME_None */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[DataAssetsLoader]")
	static FName GetRegistryRowNameByIndex(const UScriptStruct* InStruct, int32 Index);

	/*********************************************************************************************
	 * Internal Helpers
	 ********************************************************************************************* */
public:
	/** Returns true if the owner's world is in a transitional state where callbacks should be suppressed:
	 * world is tearing down, network transition in progress, or PIE is ending */
	static bool IsOwnerWorldStale(const UObject* Owner);

	/** Executes callback with PIE-safe dispatch: defers in editor with weak context safety, direct call otherwise */
	static void ExecutePIESafe(const UObject* ContextObject, TFunction<void()> Callback);

	/** Returns the current play world as UObject for weak pointer storage */
	static class UWorld* GetPlayWorld(const UObject* WorldContextObject);

	/** Cached soft property lists per struct type for GatherAllSoftPaths */
	static TMap<const UScriptStruct*, TArray<const class FSoftObjectProperty*>>& GetSoftPropsCache();

	/** Returns cached array of soft object/class properties for the given struct */
	static const TArray<const class FSoftObjectProperty*>& GetSoftProperties(const UScriptStruct* InStruct);

	/** Returns true if currently in editor mode (not packaged, not -game) */
	static bool IsEditor();
};
