// Copyright (c) Yevhenii Selivanov

#include "BmrMapTagCustomization.h"

// UE
#include "PropertyEditorModule.h"

/** The name of class to be customized: BmrMapTag */
const FName FBmrMapTagCustomization::PropertyClassName = TEXT("BmrMapTag");

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FBmrMapTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Map Tag
void FBmrMapTagCustomization::RegisterMapTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited BmrMapTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
	    PropertyClassName,
	    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBmrMapTagCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Map Tag
void FBmrMapTagCustomization::UnregisterMapTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}
