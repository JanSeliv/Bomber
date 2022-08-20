// Copyright (c) Yevhenii Selivanov

#include "AssetTypeActions_SettingsDataTable.h"
//---
#include "Data/SettingsDataTable.h"
#include "MySettingsWidgetConstructorEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

USettingsDataTableFactory::USettingsDataTableFactory()
{
	SupportedClass = USettingsDataTable::StaticClass();
}

FText USettingsDataTableFactory::GetDisplayName() const
{
	return LOCTEXT("SettingsDataTableFactory", "Settings Data Table");
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
