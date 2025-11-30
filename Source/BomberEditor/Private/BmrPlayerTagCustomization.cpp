// Copyright (c) Yevhenii Selivanov

#include "BmrPlayerTagCustomization.h"

// UE
#include "PropertyEditorModule.h"

/** The name of class to be customized: PlayerTag */
// @TODO JanSeliv 2dUuTjyT use 'FPlayerTag::StaticStruct()->GetFName()' as soon as the editor module starts referencing the runtime module
const FName FBmrPlayerTagCustomization::PropertyClassName = TEXT("PlayerTag");

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FBmrPlayerTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Players Tag
void FBmrPlayerTagCustomization::RegisterPlayersTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited PlayerTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
	    PropertyClassName,
	    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBmrPlayerTagCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Players Tag
void FBmrPlayerTagCustomization::UnregisterPlayersTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}
