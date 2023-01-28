// Copyright (c) Yevhenii Selivanov

#include "SettingTagCustomization.h"
//---
#include "Data/SettingTag.h"

/** The name of class to be customized: SettingTag */
const FName FSettingTagCustomization::PropertyClassName = FSettingTag::StaticStruct()->GetFName();

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FSettingTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Settings Tag
void FSettingTagCustomization::RegisterSettingsTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited SettingTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
		PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSettingTagCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Settings Tag
void FSettingTagCustomization::UnregisterSettingsTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}