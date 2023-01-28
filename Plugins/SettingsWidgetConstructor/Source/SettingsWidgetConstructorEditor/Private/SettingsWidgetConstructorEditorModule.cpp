// Copyright (c) Yevhenii Selivanov.

#include "SettingsWidgetConstructorEditorModule.h"
//---
#include "AssetTypeActions_SettingsDataTable.h"
#include "AssetTypeActions_SettingsWidget.h"
#include "SettingsPickerCustomization.h"
#include "SettingTagCustomization.h"
#include "FunctionPickerType/FunctionPickerCustomization.h"
//---
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FSettingsWidgetConstructorEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FSettingsWidgetConstructorEditorModule::StartupModule()
{
	RegisterPropertyCustomizations();
	RegisterSettingAssets();
}

// Called before the module is unloaded, right before the module object is destroyed
void FSettingsWidgetConstructorEditorModule::ShutdownModule()
{
	UnregisterPropertyCustomizations();
	UnregisterAssets(RegisteredAssets);
}

// Removes all custom assets from context menu
void FSettingsWidgetConstructorEditorModule::UnregisterAssets(TArray<TSharedPtr<FAssetTypeActions_Base>>& RegisteredAssets)
{
	static const FName AssetToolsModuleName = TEXT("AssetTools");
	const FAssetToolsModule* AssetToolsPtr = FModuleManager::GetModulePtr<FAssetToolsModule>(AssetToolsModuleName);
	if (!AssetToolsPtr)
	{
		return;
	}

	IAssetTools& AssetTools = AssetToolsPtr->Get();
	for (TSharedPtr<FAssetTypeActions_Base>& AssetTypeActionIt : RegisteredAssets)
	{
		if (AssetTypeActionIt.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(AssetTypeActionIt.ToSharedRef());
		}
	}
}

// Creates all customizations for custom properties
void FSettingsWidgetConstructorEditorModule::RegisterPropertyCustomizations()
{
	FFunctionPickerCustomization::RegisterFunctionPickerCustomization();
	FSettingsPickerCustomization::RegisterSettingsPickerCustomization();
	FSettingTagCustomization::RegisterSettingsTagCustomization();
}

// Removes all custom property customizations
void FSettingsWidgetConstructorEditorModule::UnregisterPropertyCustomizations()
{
	FFunctionPickerCustomization::UnregisterFunctionPickerCustomization();
	FSettingsPickerCustomization::UnregisterSettingsPickerCustomization();
	FSettingTagCustomization::UnregisterSettingsTagCustomization();
}

// Adds to context menu custom assets to be created
void FSettingsWidgetConstructorEditorModule::RegisterSettingAssets()
{
	RegisterSettingAssetsCategory();

	RegisterAsset<FAssetTypeActions_SettingsDataTable>(RegisteredAssets);
	RegisterAsset<FAssetTypeActions_SettingsWidget>(RegisteredAssets);
}

// Adds the category of this plugin to the 'Add' context menu
void FSettingsWidgetConstructorEditorModule::RegisterSettingAssetsCategory()
{
	static const FName CategoryKey = TEXT("SettingsWidgetConstructor");
	static const FText CategoryDisplayName = LOCTEXT("SettingsWidgetConstructorCategory", "Settings Widget Constructor");
	SettingsCategory = IAssetTools::Get().RegisterAdvancedAssetCategory(CategoryKey, CategoryDisplayName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSettingsWidgetConstructorEditorModule, SettingsWidgetConstructorEditor)
