// Copyright 2020 Yevhenii Selivanov.

#include "BomberEditorModule.h"
//---
#include "AttachedMeshCustomization.h"
//---
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FBomberEditorModule, BomberEditor);

#define LOCTEXT_NAMESPACE "BomberEditor"

static const FName AttachedMeshPropertyType = "AttachedMesh";

void FBomberEditorModule::StartupModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// FAttachedMesh property realizes the functionally of the SSocketChooser
	PropertyModule.RegisterCustomPropertyTypeLayout(
		AttachedMeshPropertyType,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAttachedMeshCustomization::MakeInstance)
		);

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FBomberEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(AttachedMeshPropertyType);
}

#undef LOCTEXT_NAMESPACE
