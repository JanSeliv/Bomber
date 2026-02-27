// Copyright (c) Yevhenii Selivanov

#include "DalUtilsLibrary.h"

// DAL
#include "DalPrimaryDataAsset.h"

// UE
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "GameFeatureData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DalUtilsLibrary)

// Returns the first found asset data for a UDalPrimaryDataAsset descendant matching a single registry tag
FAssetData UDalUtilsLibrary::GetAssetByRegistryTag(FName AssetTag, const FString& AssetValue)
{
	if (AssetTag.IsNone()
	    || AssetValue.IsEmpty())
	{
		return FAssetData();
	}

	TMultiMap<FName, TOptional<FString>> TagsAndValues;
	TagsAndValues.Add(AssetTag, AssetValue);

	TArray<FAssetData> AssetsData;
	GetAssetsByRegistryTags(/*out*/ AssetsData, TagsAndValues);

	return AssetsData.IsValidIndex(0) ? AssetsData[0] : FAssetData();
}

// Returns asset data for all registered UDalPrimaryDataAsset descendants matching the specified registry tags
void UDalUtilsLibrary::GetAssetsByRegistryTags(TArray<FAssetData>& OutAssetsData, const TMultiMap<FName, TOptional<FString>>& TagsAndValues)
{
	if (TagsAndValues.IsEmpty())
	{
		return;
	}

	FARFilter Filter;
	static const FTopLevelAssetPath AssetClassPath = UDalPrimaryDataAsset::StaticClass()->GetClassPathName();
	Filter.ClassPaths.Add(AssetClassPath);
	Filter.bRecursiveClasses = true;
	Filter.TagsAndValues = TagsAndValues;
	UAssetManager::Get().GetAssetRegistry().GetAssets(Filter, OutAssetsData);
}

// Returns primary asset IDs for all UDalPrimaryDataAsset descendants discovered under the game feature plugin root path
void UDalUtilsLibrary::GetAssetsInGameFeaturePlugin(TArray<FPrimaryAssetId>& OutAssetIds, const UGameFeatureData* GameFeatureData)
{
	if (!GameFeatureData)
	{
		return;
	}

	FString PluginRootPath;
	FString OutPackagePath;
	FString OutPackageName;
	FPackageName::SplitLongPackageName(GameFeatureData->GetPathName(), PluginRootPath, OutPackagePath, OutPackageName);
	PluginRootPath.RemoveFromEnd(TEXT("/"));
	if (PluginRootPath.IsEmpty())
	{
		return;
	}

	FARFilter Filter;
	static const FTopLevelAssetPath AssetClassPath = UDalPrimaryDataAsset::StaticClass()->GetClassPathName();
	Filter.ClassPaths.Add(AssetClassPath);
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add(*PluginRootPath);
	Filter.bRecursivePaths = true;

	TArray<FAssetData> FoundAssets;
	UAssetManager::Get().GetAssetRegistry().GetAssets(Filter, FoundAssets);
	for (const FAssetData& AssetData : FoundAssets)
	{
		OutAssetIds.Emplace(AssetData.GetPrimaryAssetId());
	}
}
