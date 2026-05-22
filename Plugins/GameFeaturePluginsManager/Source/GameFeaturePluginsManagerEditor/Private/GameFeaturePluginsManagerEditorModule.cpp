// Copyright (c) Yevhenii Selivanov.

#include "GameFeaturePluginsManagerEditorModule.h"

// UE
#include "Modules/ModuleManager.h"

// Called right after the module DLL has been loaded and the module object has been created
void FGameFeaturePluginsManagerEditorModule::StartupModule()
{
}

// Called before the module is unloaded, right before the module object is destroyed
void FGameFeaturePluginsManagerEditorModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameFeaturePluginsManagerEditorModule, GameFeaturePluginsManagerEditor)