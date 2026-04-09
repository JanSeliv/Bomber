// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameplayTagsEditorModule.h"

/**
 * Is customized to show only NMM state options in the tag picker.
 */
class NEWMAINMENUEDITOR_API FNmmStateTagCustomization : public FGameplayTagCustomizationPublic
{
public:
	/** Is used to load and unload the Property Editor Module. */
	inline static const FName PropertyEditorModule = TEXT("PropertyEditor");

	/** The name of class to be customized: NmmStateTag. */
	static const FName PropertyClassName;

	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** Creates customization for the NMM State Tag. */
	static void RegisterNmmStateTagCustomization();

	/** Removes customization for the NMM State Tag. */
	static void UnregisterNmmStateTagCustomization();
};
