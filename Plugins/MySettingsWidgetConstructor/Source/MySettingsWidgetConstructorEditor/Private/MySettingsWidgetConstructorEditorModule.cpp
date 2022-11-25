// Copyright (c) Yevhenii Selivanov.

#include "MySettingsWidgetConstructorEditorModule.h"
//---
#include "SettingsPickerCustomization.h"
#include "AssetTypeActions_SettingsDataTable.h"
#include "AssetTypeActions_SettingsWidget.h"
//---
#include "GameplayTagsEditorModule.h"
#include "EditorUtilsLibrary.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMySettingsWidgetConstructorEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMySettingsWidgetConstructorEditorModule::StartupModule()
{
	RegisterPropertyCustomizations();
	RegisterSettingAssets();
}

// Called before the module is unloaded, right before the module object is destroyed
void FMySettingsWidgetConstructorEditorModule::ShutdownModule()
{
	UnregisterPropertyCustomizations();
	UEditorUtilsLibrary::UnregisterAssets(RegisteredAssets);
}

// Creates all customizations for custom properties
void FMySettingsWidgetConstructorEditorModule::RegisterPropertyCustomizations()
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

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes all custom property customizations
void FMySettingsWidgetConstructorEditorModule::UnregisterPropertyCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(FSettingsPickerCustomization::PropertyClassName);
	PropertyModule.UnregisterCustomPropertyTypeLayout(SettingTagStructureName);
}

// Adds to context menu custom assets to be created
void FMySettingsWidgetConstructorEditorModule::RegisterSettingAssets()
{
	RegisterSettingsCategory();

	UEditorUtilsLibrary::RegisterAsset<FAssetTypeActions_SettingsDataTable>(RegisteredAssets);
	UEditorUtilsLibrary::RegisterAsset<FAssetTypeActions_SettingsWidget>(RegisteredAssets);
}

// Adds the category of this plugin to the 'Add' context menu
void FMySettingsWidgetConstructorEditorModule::RegisterSettingsCategory()
{
	static const FName CategoryKey = TEXT("MySettingsWidgetConstructor");
	static const FText CategoryDisplayName = LOCTEXT("MySettingsWidgetConstructorCategory", "Settings Widget Constructor");
	SettingsCategory = IAssetTools::Get().RegisterAdvancedAssetCategory(CategoryKey, CategoryDisplayName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMySettingsWidgetConstructorEditorModule, MySettingsWidgetConstructorEditor)
