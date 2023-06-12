// Copyright (c) Yevhenii Selivanov.

#include "MyUtilsModule.h"

#define LOCTEXT_NAMESPACE "FMyUtilsModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMyUtilsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

// Called before the module is unloaded, right before the module object is destroyed
void FMyUtilsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyUtilsModule, MyUtils)
