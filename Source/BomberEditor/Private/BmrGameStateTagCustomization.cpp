// Copyright (c) Yevhenii Selivanov

#include "BmrGameStateTagCustomization.h"

// UE
#include "PropertyEditorModule.h"

/** The name of class to be customized: BmrGameStateTag */
// @TODO JanSeliv 2dUuTjyT use 'FBmrGameStateTag::StaticStruct()->GetFName()' as soon as the editor module starts referencing the runtime module
const FName FBmrGameStateTagCustomization::PropertyClassName = TEXT("BmrGameStateTag");

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FBmrGameStateTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Game State Tag
void FBmrGameStateTagCustomization::RegisterGameStateTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited BmrGameStateTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
	    PropertyClassName,
	    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBmrGameStateTagCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Game State Tag
void FBmrGameStateTagCustomization::UnregisterGameStateTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}
