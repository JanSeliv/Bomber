// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/AssetManager.h"

#include "GfpmAssetManager.generated.h"

/**
 * Asset manager that lets World Partition content of Game Feature Plugin (its External Data Layers) cook into separate mod.
 * Project enables it by setting its AssetManagerClassName to this class, or by deriving its own asset manager from it.
 */
UCLASS(Config = "GameFeaturePluginsManager")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmAssetManager : public UAssetManager
{
	GENERATED_BODY()

protected:
	/** Keeps every game feature plugin content out of project cook so each ships as separate removable mod, project opts in. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	bool bExcludeGameFeaturesFromProjectCook;

#if WITH_EDITOR
public:
	/** External Data Layer host world package names given plugin injects into, discovered from its cooked external-actor layout via asset registry.
	 * Not Blueprint-exposed: editor-only cook utility returning FName package paths. */
	static TArray<FName> GetExternalDataLayerHostWorlds(const FString& PluginName);

protected:
	/** Adds External Data Layer host worlds cooked plugin injects into to cook set, so their World Partition generators run and plugin content is produced into mod.
	 * Not Blueprint-exposed: editor-only cook-time virtual parent does not expose. */
	virtual void ModifyDLCCook(const FString& DLCName, TConstArrayView<const class ITargetPlatform*> TargetPlatforms, TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook) override;

	/** Clears project cook result of those same host worlds, so World Partition generator re-runs instead of being skipped as already cooked.
	 * Not Blueprint-exposed: editor-only cook-time virtual parent does not expose. */
	virtual void ModifyDLCBasePackages(const class ITargetPlatform* TargetPlatform, TArray<FName>& PlatformBasedPackages, TSet<FName>& PackagesToClearResults) const override;

	/** Keeps opted-in game feature plugin content out of project cook, so plugins ship only as separate cooked mods.
	 * Not Blueprint-exposed: editor-only cook-time virtual parent does not expose. */
	virtual void ModifyCook(TConstArrayView<const class ITargetPlatform*> TargetPlatforms, TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook) override;

private:
	/** External Data Layer host world package names cooked mod injects into, self-discovered from plugin content, command-line -Map= as fallback. */
	static TArray<FName> ModCookHostWorlds();
#endif // WITH_EDITOR
};
