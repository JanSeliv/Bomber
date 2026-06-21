// Copyright (c) Yevhenii Selivanov

#include "GfpmAssetManager.h"

#if WITH_EDITOR
// GFPM
#include "GfpmUtils.h"

// UE
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"
#include "GameFeatureData.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "WorldPartition/DataLayer/ExternalDataLayerHelper.h"
#include "WorldPartition/DataLayer/ExternalDataLayerUID.h"
#endif // WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(GfpmAssetManager)

#if WITH_EDITOR
namespace GfpmAssetManagerInternal
{
	// Walks up an External Data Layer external-actor relative path to the host world package it belongs to, longest prefix registered as a world
	static FName ResolveHostWorldPackage(const IAssetRegistry& AssetRegistry, FStringView RelativeActorPath)
	{
		const FTopLevelAssetPath WorldClassPath = UWorld::StaticClass()->GetClassPathName();
		FString Candidate(RelativeActorPath);
		while (true)
		{
			int32 SlashIndex = INDEX_NONE;
			if (!Candidate.FindLastChar(TEXT('/'), SlashIndex) || SlashIndex <= 0)
			{
				// Reached mount root with no world found
				return NAME_None;
			}
			Candidate.LeftInline(SlashIndex);

			TArray<FAssetData> Assets;
			AssetRegistry.GetAssetsByPackageName(FName(*Candidate), Assets, /*bIncludeOnlyOnDiskAssets*/true);
			const bool bIsWorld = Assets.ContainsByPredicate([&WorldClassPath](const FAssetData& Asset) { return Asset.AssetClassPath == WorldClassPath; });
			if (bIsWorld)
			{
				return FName(*Candidate);
			}
		}
	}
}

// External Data Layer host world package names given plugin injects into, discovered from its cooked external-actor layout via asset registry
TArray<FName> UGfpmAssetManager::GetExternalDataLayerHostWorlds(const FString& PluginName)
{
	TArray<FName> HostWorlds;
	if (PluginName.IsEmpty())
	{
		// No plugin specified, nothing to discover
		return HostWorlds;
	}

	const IAssetRegistry& AssetRegistry = Get().GetAssetRegistry();

	// Read External Data Layer UIDs the plugin declares from its Game Feature Data asset registry tag, no asset load needed
	FARFilter GameFeatureDataFilter;
	GameFeatureDataFilter.PackagePaths.Add(*FString::Printf(TEXT("/%s"), *PluginName));
	GameFeatureDataFilter.bRecursivePaths = true;
	GameFeatureDataFilter.ClassPaths.Add(UGameFeatureData::StaticClass()->GetClassPathName());
	GameFeatureDataFilter.bRecursiveClasses = true;
	GameFeatureDataFilter.bIncludeOnlyOnDiskAssets = true;
	TArray<FAssetData> GameFeatureDataAssets;
	AssetRegistry.GetAssets(GameFeatureDataFilter, GameFeatureDataAssets);

	TArray<FExternalDataLayerUID> ExternalDataLayerUIDs;
	for (const FAssetData& GameFeatureDataAsset : GameFeatureDataAssets)
	{
		FExternalDataLayerHelper::GetExternalDataLayerUIDs(GameFeatureDataAsset, ExternalDataLayerUIDs);
	}

	TSet<FName> Worlds;
	for (const FExternalDataLayerUID& ExternalDataLayerUID : ExternalDataLayerUIDs)
	{
		FString ActorsRootPath;
		if (!FExternalDataLayerHelper::BuildExternalDataLayerActorsRootPath(PluginName, ExternalDataLayerUID, ActorsRootPath))
		{
			// Invalid UID or mount point, nothing to enumerate
			continue;
		}

		FARFilter ActorsFilter;
		ActorsFilter.PackagePaths.Add(*ActorsRootPath);
		ActorsFilter.bRecursivePaths = true;
		ActorsFilter.bIncludeOnlyOnDiskAssets = true;
		TArray<FAssetData> ExternalActorAssets;
		AssetRegistry.GetAssets(ActorsFilter, ExternalActorAssets);

		for (const FAssetData& ExternalActorAsset : ExternalActorAssets)
		{
			const FString ExternalActorPackage = ExternalActorAsset.PackageName.ToString();
			const FStringView RelativeActorPath = FExternalDataLayerHelper::GetRelativeExternalActorPackagePath(ExternalActorPackage);
			if (RelativeActorPath.IsEmpty())
			{
				// Package not under the External Data Layer external-actor layout, skip
				continue;
			}

			const FName HostWorld = GfpmAssetManagerInternal::ResolveHostWorldPackage(AssetRegistry, RelativeActorPath);
			if (!HostWorld.IsNone())
			{
				Worlds.Add(HostWorld);
			}
		}
	}

	HostWorlds = Worlds.Array();
	return HostWorlds;
}

// External Data Layer host world package names cooked mod injects into, self-discovered from plugin content, command-line -Map= as fallback
TArray<FName> UGfpmAssetManager::ModCookHostWorlds()
{
	FString PluginName;
	if (!FParse::Value(FCommandLine::Get(), TEXT("DLCName="), PluginName) || PluginName.IsEmpty())
	{
		// Not mod cook, nothing to inject
		return {};
	}

	TArray<FName> HostWorlds = GetExternalDataLayerHostWorlds(PluginName);
	if (HostWorlds.IsEmpty())
	{
		// Self-discovery found nothing, fall back to host worlds passed explicitly via cook command line
		FString MapList;
		if (FParse::Value(FCommandLine::Get(), TEXT("Map="), MapList) && !MapList.IsEmpty())
		{
			TArray<FString> MapArray;
			MapList.ParseIntoArray(MapArray, TEXT("+"), true);
			for (const FString& Map : MapArray)
			{
				HostWorlds.Add(FName(*Map));
			}
		}
	}
	return HostWorlds;
}

// Adds External Data Layer host worlds cooked plugin injects into to cook set, so their World Partition generators run and plugin content is produced into mod
void UGfpmAssetManager::ModifyDLCCook(const FString& DLCName, TConstArrayView<const ITargetPlatform*> TargetPlatforms, TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook)
{
	Super::ModifyDLCCook(DLCName, TargetPlatforms, PackagesToCook, PackagesToNeverCook);

	const TArray<FName> HostWorlds = ModCookHostWorlds();
	for (const FName HostWorld : HostWorlds)
	{
		PackagesToCook.AddUnique(HostWorld);
	}
}

// Clears project cook result of those same host worlds, so World Partition generator re-runs instead of being skipped as already cooked
void UGfpmAssetManager::ModifyDLCBasePackages(const ITargetPlatform* TargetPlatform, TArray<FName>& PlatformBasedPackages, TSet<FName>& PackagesToClearResults) const
{
	Super::ModifyDLCBasePackages(TargetPlatform, PlatformBasedPackages, PackagesToClearResults);

	const TArray<FName> HostWorlds = ModCookHostWorlds();
	if (HostWorlds.IsEmpty())
	{
		// No External Data Layer host worlds requested by this cook, nothing to re-cook
		return;
	}

	const TSet<FName> HostWorldSet(HostWorlds);
	for (const FName BasePackageFile : PlatformBasedPackages)
	{
		FString PackageName;
		if (!FPackageName::TryConvertFilenameToLongPackageName(BasePackageFile.ToString(), PackageName))
		{
			// Base entry already a long package name when filename conversion does not apply
			PackageName = BasePackageFile.ToString();
		}

		const FName PackageFName(*PackageName);
		if (HostWorldSet.Contains(PackageFName))
		{
			// ClearCookResultsForPackages matches by package name, not by the file name passed in
			PackagesToClearResults.Add(PackageFName);
		}
	}
}

// Keeps opted-in game feature plugin content out of project cook, so plugins ship only as separate cooked mods
void UGfpmAssetManager::ModifyCook(TConstArrayView<const ITargetPlatform*> TargetPlatforms, TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook)
{
	Super::ModifyCook(TargetPlatforms, PackagesToCook, PackagesToNeverCook);

	FString DLCName;
	const bool bIsDLCCook = FParse::Value(FCommandLine::Get(), TEXT("DLCName="), DLCName) && !DLCName.IsEmpty();
	const bool bExcludeFromProjectCook = bExcludeGameFeaturesFromProjectCook || FParse::Param(FCommandLine::Get(), TEXT("GfpmExcludeGameFeatures"));
	if (!bIsDLCCook && !bExcludeFromProjectCook)
	{
		// Project cook and project ships game features inline, nothing to exclude
		return;
	}

	const TArray<FString> RegisteredPlugins = UGfpmUtils::GetAllRegisteredGameFeaturePlugins();
	if (bIsDLCCook && !RegisteredPlugins.Contains(DLCName))
	{
		// Not game feature plugin DLC, leave standard behavior
		return;
	}

	// Project cook leaves every plugin out, plugin DLC cook keeps only its own content so each mod ships isolated and other plugins do not bloat it
	const IAssetRegistry& AssetRegistry = GetAssetRegistry();
	for (const FString& RegisteredPlugin : RegisteredPlugins)
	{
		if (bIsDLCCook && RegisteredPlugin == DLCName)
		{
			// Keep DLC plugin's own content
			continue;
		}

		FARFilter PluginFilter;
		PluginFilter.PackagePaths.Add(FName(*FString::Printf(TEXT("/%s"), *RegisteredPlugin)));
		PluginFilter.bRecursivePaths = true;
		PluginFilter.bIncludeOnlyOnDiskAssets = true;
		TArray<FAssetData> PluginAssets;
		AssetRegistry.GetAssets(PluginFilter, PluginAssets);
		for (const FAssetData& PluginAsset : PluginAssets)
		{
			PackagesToNeverCook.AddUnique(PluginAsset.PackageName);
		}
	}
}
#endif // WITH_EDITOR
