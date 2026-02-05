// Copyright (c) Yevhenii Selivanov.

#include "BomberEditorModule.h"

// Bomber
#include "BmrGameStateTagCustomization.h"
#include "BmrPlayerTagCustomization.h"
#include "BmrPowerupTagCustomization.h"

// UE
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FBomberEditorModule, BomberEditor);

DEFINE_LOG_CATEGORY(LogBomberEditor);

// Called right after the module DLL has been loaded and the module object has been created
void FBomberEditorModule::StartupModule()
{
	FBmrGameStateTagCustomization::RegisterGameStateTagCustomization();
	FBmrPlayerTagCustomization::RegisterPlayersTagCustomization();
	FBmrPowerupTagCustomization::RegisterPowerupTagCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FBomberEditorModule::ShutdownModule()
{
	FBmrGameStateTagCustomization::UnregisterGameStateTagCustomization();
	FBmrPlayerTagCustomization::UnregisterPlayersTagCustomization();
	FBmrPowerupTagCustomization::UnregisterPowerupTagCustomization();
}
