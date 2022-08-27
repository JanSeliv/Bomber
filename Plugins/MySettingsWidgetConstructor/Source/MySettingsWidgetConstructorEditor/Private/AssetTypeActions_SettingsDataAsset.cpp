// Copyright (c) Yevhenii Selivanov

#include "AssetTypeActions_SettingsDataAsset.h"
//---
#include "MySettingsWidgetConstructorEditorModule.h"
#include "Data/SettingsDataAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

USettingsDataAssetFactory::USettingsDataAssetFactory()
{
	bEditAfterNew = true;
	bCreateNew = true;
	SupportedClass = USettingsDataAsset::StaticClass();
}

FText USettingsDataAssetFactory::GetDisplayName() const
{
	return LOCTEXT("SettingsDataAsset", "Settings Data Asset");
}

UObject* USettingsDataAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(InClass && InClass->IsChildOf<USettingsDataAsset>());
	return NewObject<USettingsDataAsset>(InParent, InClass, InName, Flags);
}

FText FAssetTypeActions_SettingsDataAsset::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SettingsDataAsset", "Settings Data Asset");
}

FColor FAssetTypeActions_SettingsDataAsset::GetTypeColor() const
{
	constexpr FColor BrownColor(139.f, 69.f, 19.f);
	return BrownColor;
}

UClass* FAssetTypeActions_SettingsDataAsset::GetSupportedClass() const
{
	return USettingsDataAsset::StaticClass();
}

uint32 FAssetTypeActions_SettingsDataAsset::GetCategories()
{
	return FMySettingsWidgetConstructorEditorModule::SettingsCategory;
}

#undef LOCTEXT_NAMESPACE
