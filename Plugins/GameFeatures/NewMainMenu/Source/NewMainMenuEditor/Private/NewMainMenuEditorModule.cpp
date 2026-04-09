// Copyright (c) Yevhenii Selivanov.

#include "NewMainMenuEditorModule.h"

// NMM Editor
#include "NmmStateTagCustomization.h"

// UE
#include "Modules/ModuleManager.h"

// Called right after the module DLL has been loaded and the module object has been created
void FNewMainMenuEditorModule::StartupModule()
{
	FNmmStateTagCustomization::RegisterNmmStateTagCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FNewMainMenuEditorModule::ShutdownModule()
{
	FNmmStateTagCustomization::UnregisterNmmStateTagCustomization();
}

IMPLEMENT_MODULE(FNewMainMenuEditorModule, NewMainMenuEditor)
