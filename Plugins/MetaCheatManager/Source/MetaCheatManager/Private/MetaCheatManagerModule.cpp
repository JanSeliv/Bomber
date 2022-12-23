// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaCheatManagerModule.h"

#define LOCTEXT_NAMESPACE "FMetaCheatManagerModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMetaCheatManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

// Called before the module is unloaded, right before the module object is destroyed
void FMetaCheatManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMetaCheatManagerModule, MetaCheatManager)
