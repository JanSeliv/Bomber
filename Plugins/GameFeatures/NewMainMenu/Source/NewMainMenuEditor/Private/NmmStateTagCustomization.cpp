// Copyright (c) Yevhenii Selivanov

#include "NmmStateTagCustomization.h"

// NMM
#include "Data/NmmStateTag.h"

// UE
#include "PropertyEditorModule.h"

// The name of class to be customized: NmmStateTag
const FName FNmmStateTagCustomization::PropertyClassName = FNmmStateTag::StaticStruct()->GetFName();

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FNmmStateTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the NMM State Tag
void FNmmStateTagCustomization::RegisterNmmStateTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited NmmStateTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
	    PropertyClassName,
	    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNmmStateTagCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the NMM State Tag
void FNmmStateTagCustomization::UnregisterNmmStateTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}
