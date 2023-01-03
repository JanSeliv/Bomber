// Copyright (c) Yevhenii Selivanov

#include "AssetTypeActions_SettingsWidget.h"
//---
#include "MySettingsWidgetConstructorEditorModule.h"
#include "UI/SettingsWidget.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

USettingsWidgetFactory::USettingsWidgetFactory()
{
	SettingsWidgetClassInternal = USettingsWidget::StaticClass();
}

FText USettingsWidgetFactory::GetDisplayName() const
{
	return LOCTEXT("SettingsWidget", "Settings Widget");
}

TSubclassOf<UUserWidget> USettingsWidgetFactory::GetWidgetClass() const
{
	check(SettingsWidgetClassInternal);
	return SettingsWidgetClassInternal;
}

FText FAssetTypeActions_SettingsWidget::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SettingsWidget", "Settings Widget");
}

FColor FAssetTypeActions_SettingsWidget::GetTypeColor() const
{
	constexpr FColor BrownColor(139.f, 69.f, 19.f);
	return BrownColor;
}

uint32 FAssetTypeActions_SettingsWidget::GetCategories()
{
	return FMySettingsWidgetConstructorEditorModule::SettingsCategory;
}

#undef LOCTEXT_NAMESPACE
