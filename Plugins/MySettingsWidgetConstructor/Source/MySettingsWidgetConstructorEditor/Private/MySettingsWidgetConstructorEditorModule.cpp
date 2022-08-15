// Copyright (c) Yevhenii Selivanov.

#include "MySettingsWidgetConstructorEditorModule.h"
//---
#include "SettingsPickerCustomization.h"
//---
#include "GameplayTagsEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMySettingsWidgetConstructorEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMySettingsWidgetConstructorEditorModule::StartupModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Is customized to show only selected in-game option
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FSettingsPickerCustomization::PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSettingsPickerCustomization::MakeInstance)
	);

	// Use default GameplayTag customization for inherited SettingTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
		SettingTagStructureName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayTagCustomizationPublic::MakeInstance)
	);
}

// Called before the module is unloaded, right before the module object is destroyed
void FMySettingsWidgetConstructorEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(FSettingsPickerCustomization::PropertyClassName);
	PropertyModule.UnregisterCustomPropertyTypeLayout(SettingTagStructureName);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMySettingsWidgetConstructorEditorModule, MySettingsWidgetConstructorEditor)
