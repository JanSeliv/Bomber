// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameplayTagsEditorModule.h"

/**
 * Is customized to show only selected in-game option.
 */
class SETTINGSWIDGETCONSTRUCTOREDITOR_API FSettingTagCustomization : public FGameplayTagCustomizationPublic
{
public:
	/** Is used to load and unload the Property Editor Module. */
	inline static const FName PropertyEditorModule = TEXT("PropertyEditor");

	/** The name of class to be customized: SettingTag. */
	static const FName PropertyClassName;

	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** Creates customization for the Settings Tag. */
	static void RegisterSettingsTagCustomization();

	/** Removes customization for the Settings Tag. */
	static void UnregisterSettingsTagCustomization();
};
