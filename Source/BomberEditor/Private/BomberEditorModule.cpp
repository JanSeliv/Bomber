// Copyright (c) Yevhenii Selivanov.

#include "BomberEditorModule.h"
//---
#include "AttachedMeshCustomization.h"
#include "PlayerTagCustomization.h"
//---
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FBomberEditorModule, BomberEditor);

DEFINE_LOG_CATEGORY(LogBomberEditor);

// Called right after the module DLL has been loaded and the module object has been created
void FBomberEditorModule::StartupModule()
{
	FAttachedMeshCustomization::RegisterAttachedMeshCustomization();
	FPlayerTagCustomization::RegisterPlayersTagCustomization();
}

// Called before the module is unloaded, right before the module object is destroyed
void FBomberEditorModule::ShutdownModule()
{
	FAttachedMeshCustomization::UnregisterAttachedMeshCustomization();
	FPlayerTagCustomization::UnregisterPlayersTagCustomization();
}
