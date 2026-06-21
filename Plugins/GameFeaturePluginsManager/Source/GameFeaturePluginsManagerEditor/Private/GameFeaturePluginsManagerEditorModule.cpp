// Copyright (c) Yevhenii Selivanov.

#include "GameFeaturePluginsManagerEditorModule.h"

// GFPM
#include "GfpmAssetManager.h"

// UE
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "GameFeaturesSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

// Called right after the module DLL has been loaded and the module object has been created
void FGameFeaturePluginsManagerEditorModule::StartupModule()
{
	UEngine* EngineDefaults = GetMutableDefault<UEngine>();
	const FSoftClassPath ConfiguredPath = EngineDefaults->AssetManagerClassName;

	if (ConfiguredPath.IsValid() && ConfiguredPath != FSoftClassPath(UAssetManager::StaticClass()))
	{
		// Project or other plugin already configured custom asset manager, keep it
		UE_LOG(LogGameFeatures, Display, TEXT("%hs: asset manager '%s' already configured, GFPM keeps it"), __FUNCTION__, *ConfiguredPath.ToString());
		return;
	}

	// Asset manager class is eager globalconfig engine resolves before plugin ini layers apply, so set GFPM one at module startup before engine creates singleton
	const FSoftClassPath GfpmAssetManagerPath(UGfpmAssetManager::StaticClass());
	const FString GfpmAssetManagerPathStr = GfpmAssetManagerPath.ToString();
	EngineDefaults->AssetManagerClassName = GfpmAssetManagerPath;
	GConfig->SetString(TEXT("/Script/Engine.Engine"), TEXT("AssetManagerClassName"), *GfpmAssetManagerPathStr, GEngineIni);
	UE_LOG(LogGameFeatures, Display, TEXT("%hs: no custom asset manager configured, GFPM installs '%s'"), __FUNCTION__, *GfpmAssetManagerPathStr);
}

// Called before the module is unloaded, right before the module object is destroyed
void FGameFeaturePluginsManagerEditorModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameFeaturePluginsManagerEditorModule, GameFeaturePluginsManagerEditor)