// Copyright (c) Yevhenii Selivanov.

#include "MySettingsWidgetConstructorEditorModule.h"
//---
#include "SettingsPickerCustomization.h"
#include "AssetTypeActions_SettingsDataAsset.h"
#include "AssetTypeActions_SettingsDataTable.h"
#include "AssetTypeActions_SettingsWidget.h"
//---
#include "GameplayTagsEditorModule.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FMySettingsWidgetConstructorEditorModule"

// Called right after the module DLL has been loaded and the module object has been created
void FMySettingsWidgetConstructorEditorModule::StartupModule()
{
	RegisterPropertyCustomizations();
	RegisterAssets();
}

// Called before the module is unloaded, right before the module object is destroyed
void FMySettingsWidgetConstructorEditorModule::ShutdownModule()
{
	UnregisterPropertyCustomizations();
	UnregisterAssets();
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
void FMySettingsWidgetConstructorEditorModule::RegisterAssets()
{
	if (!FModuleManager::Get().IsModuleLoaded(AssetToolsModule))
	{
		return;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetToolsModule).Get();

	RegisterSettingsCategory(AssetTools);
	RegisterSettingsDataTable(AssetTools);
	RegisterSettingsDataAsset(AssetTools);
	RegisterSettingsWidget(AssetTools);
}

// Removes all custom assets from context menu
void FMySettingsWidgetConstructorEditorModule::UnregisterAssets()
{
	if (!FModuleManager::Get().IsModuleLoaded(AssetToolsModule))
	{
		return;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetToolsModule).Get();

	for (TSharedPtr<FAssetTypeActions_Base>& AssetTypeActionIt : RegisteredAssets)
	{
		if (AssetTypeActionIt.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(AssetTypeActionIt.ToSharedRef());
		}
	}
}

// Adds the category of this plugin to the 'Add' context menu
void FMySettingsWidgetConstructorEditorModule::RegisterSettingsCategory(IAssetTools& AssetTools)
{
	static const FName CategoryKey = TEXT("MySettingsWidgetConstructor");
	static const FText CategoryDisplayName = LOCTEXT("MySettingsWidgetConstructorCategory", "My Settings Widget Constructor");
	SettingsCategory = AssetTools.RegisterAdvancedAssetCategory(CategoryKey, CategoryDisplayName);
}

// Adds the 'Settings Data Table' asset to the context menu
void FMySettingsWidgetConstructorEditorModule::RegisterSettingsDataTable(IAssetTools& AssetTools)
{
	TSharedPtr<FAssetTypeActions_SettingsDataTable> SettingsDataTableAction = MakeShared<FAssetTypeActions_SettingsDataTable>();
	AssetTools.RegisterAssetTypeActions(SettingsDataTableAction.ToSharedRef());
	RegisteredAssets.Emplace(MoveTemp(SettingsDataTableAction));
}

// Adds the 'Settings Data Asset' to the context menu
void FMySettingsWidgetConstructorEditorModule::RegisterSettingsDataAsset(IAssetTools& AssetTools)
{
	TSharedPtr<FAssetTypeActions_SettingsDataAsset> SettingsDataAssetAction = MakeShared<FAssetTypeActions_SettingsDataAsset>();
	AssetTools.RegisterAssetTypeActions(SettingsDataAssetAction.ToSharedRef());
	RegisteredAssets.Emplace(MoveTemp(SettingsDataAssetAction));
}

// Adds the 'Settings Widget' to the context menu
void FMySettingsWidgetConstructorEditorModule::RegisterSettingsWidget(IAssetTools& AssetTools)
{
	TSharedPtr<FAssetTypeActions_SettingsWidget> SettingsWidgetAction = MakeShared<FAssetTypeActions_SettingsWidget>();
	AssetTools.RegisterAssetTypeActions(SettingsWidgetAction.ToSharedRef());
	RegisteredAssets.Emplace(MoveTemp(SettingsWidgetAction));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMySettingsWidgetConstructorEditorModule, MySettingsWidgetConstructorEditor)
