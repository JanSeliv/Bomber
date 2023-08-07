// Copyright (c) Yevhenii Selivanov

#include "PlayerTagCustomization.h"
//---
#include "PropertyEditorModule.h"

/** The name of class to be customized: PlayerTag */
// @TODO JanSeliv 2dUuTjyT use 'FPlayerTag::StaticStruct()->GetFName()' as soon as the editor module starts referencing the runtime module
const FName FPlayerTagCustomization::PropertyClassName = TEXT("PlayerTag");

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FPlayerTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Players Tag
void FPlayerTagCustomization::RegisterPlayersTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited PlayerTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
		PropertyClassName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPlayerTagCustomization::MakeInstance)
		);

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Players Tag
void FPlayerTagCustomization::UnregisterPlayersTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}
