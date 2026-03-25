// Copyright (c) Yevhenii Selivanov

#include "BmrGameDifficultyTagCustomization.h"

// UE
#include "PropertyEditorModule.h"

/** The name of class to be customized: BmrGameDifficultyTag */
// @TODO JanSeliv 2dUuTjyT use 'FBmrGameDifficultyTag::StaticStruct()->GetFName()' as soon as the editor module starts referencing the runtime module
const FName FBmrGameDifficultyTagCustomization::PropertyClassName = TEXT("BmrGameDifficultyTag");

// Makes a new instance of this detail layout class for a specific detail view requesting it
TSharedRef<IPropertyTypeCustomization> FBmrGameDifficultyTagCustomization::MakeInstance()
{
	return FGameplayTagCustomizationPublic::MakeInstance();
}

// Creates customization for the Game Difficulty Tag
void FBmrGameDifficultyTagCustomization::RegisterGameDifficultyTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	// Use default GameplayTag customization for inherited BmrGameDifficultyTag to show Tags list
	PropertyModule.RegisterCustomPropertyTypeLayout(
	    PropertyClassName,
	    FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBmrGameDifficultyTagCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

// Removes customization for the Game Difficulty Tag
void FBmrGameDifficultyTagCustomization::UnregisterGameDifficultyTagCustomization()
{
	if (!FModuleManager::Get().IsModuleLoaded(PropertyEditorModule))
	{
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(PropertyEditorModule);

	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyClassName);
}