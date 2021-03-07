// Copyright 2021 Yevhenii Selivanov.

#include "BomberEditorModule.h"
//---
#include "AttachedMeshCustomization.h"
#include "MorphDataCustomization.h"
#include "SettingsFunctionCustomization.h"
//---
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FBomberEditorModule, BomberEditor);

static const FName PropertyEditorModule = "PropertyEditor";
static const FName AttachedMeshProperty = "AttachedMesh";
static const FName SettingsFunctionProperty = "SettingsFunction";
static const FName MorphDataProperty = "MorphData";

void FBomberEditorModule::StartupModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// FAttachedMesh property realizes the functionally of the SSocketChooser
	PropertyModule.RegisterCustomPropertyTypeLayout(
		AttachedMeshProperty,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAttachedMeshCustomization::MakeInstance)
		);

	// Allows to choose ufunction
	PropertyModule.RegisterCustomPropertyTypeLayout(
		SettingsFunctionProperty,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSettingsFunctionCustomization::MakeInstance)
		);

	// Allows to choose a morph
	PropertyModule.RegisterCustomPropertyTypeLayout(
		MorphDataProperty,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMorphDataCustomization::MakeInstance)
		);

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FBomberEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(AttachedMeshProperty);
	PropertyModule.UnregisterCustomPropertyTypeLayout(SettingsFunctionProperty);
	PropertyModule.UnregisterCustomPropertyTypeLayout(MorphDataProperty);
}
