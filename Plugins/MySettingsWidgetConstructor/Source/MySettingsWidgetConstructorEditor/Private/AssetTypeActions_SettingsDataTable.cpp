// Copyright (c) Yevhenii Selivanov

#include "AssetTypeActions_SettingsDataTable.h"
//---
#include "Data/SettingsDataTable.h"
#include "MySettingsWidgetConstructorEditorModule.h"
//---
#include "EditorFramework/AssetImportData.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

USettingsDataTableFactory::USettingsDataTableFactory()
{
	SupportedClass = USettingsDataTable::StaticClass();
}

FText USettingsDataTableFactory::GetDisplayName() const
{
	return LOCTEXT("SettingsDataTableFactory", "Settings Data Table");
}

UObject* USettingsDataTableFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UObject* NewObject = Super::FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn);

	ImportDefaultSettingsDataTable(NewObject);

	return NewObject;
}

// Imports default data into new Settings Data Table
void USettingsDataTableFactory::ImportDefaultSettingsDataTable(UObject* NewSettingDataTable)
{
	USettingsDataTable* SettingsDataTable = CastChecked<USettingsDataTable>(NewSettingDataTable);
	UAssetImportData* AssetImportData = SettingsDataTable->AssetImportData;
	check(AssetImportData);

	// Find the .json
	static FString DataTableGlobalDir = TEXT("");
	if (DataTableGlobalDir.IsEmpty())
	{
		const FString ThisPluginName = TEXT("MySettingsWidgetConstructor");
		const FString DataTableRelativeDir = TEXT("Config/BaseSettingsDataTable.json");
		const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(ThisPluginName);
		checkf(ThisPlugin, TEXT("ASSERT: '%s' plugin is not found"), *ThisPluginName);
		DataTableGlobalDir = ThisPlugin->GetBaseDir() / DataTableRelativeDir;
	}

	// Import the .json
	AssetImportData->UpdateFilenameOnly(DataTableGlobalDir);
	Reimport(SettingsDataTable);

	// Clear import path to prevent reimporting default data for already created table
	AssetImportData->SourceData = FAssetImportInfo();
}

FText FAssetTypeActions_SettingsDataTable::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SettingsDataTable", "Settings Data Table");
}

FColor FAssetTypeActions_SettingsDataTable::GetTypeColor() const
{
	constexpr FColor BrownColor(139.f, 69.f, 19.f);
	return BrownColor;
}

UClass* FAssetTypeActions_SettingsDataTable::GetSupportedClass() const
{
	return USettingsDataTable::StaticClass();
}

uint32 FAssetTypeActions_SettingsDataTable::GetCategories()
{
	return FMySettingsWidgetConstructorEditorModule::SettingsCategory;
}

#undef LOCTEXT_NAMESPACE
