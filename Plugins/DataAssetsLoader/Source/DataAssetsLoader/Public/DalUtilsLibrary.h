// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "DalUtilsLibrary.generated.h"

/**
 * Blueprint utility class for Data Assets Loader.
 * Provides helper functions for querying data assets from the asset registry.
 */
UCLASS()
class DATAASSETSLOADER_API UDalUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

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
};
