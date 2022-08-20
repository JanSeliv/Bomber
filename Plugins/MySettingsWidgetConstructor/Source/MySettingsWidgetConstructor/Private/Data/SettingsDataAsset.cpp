// Copyright (c) Yevhenii Selivanov.

#include "Data/SettingsDataAsset.h"
//---
#include "UI/SettingSubWidget.h"

// Default constructor
USettingsDataAsset::USettingsDataAsset()
{
	ButtonClassInternal = USettingButton::StaticClass();
	CheckboxClassInternal = USettingCheckbox::StaticClass();
	ComboboxClassInternal = USettingCombobox::StaticClass();
	SliderClassInternal = USettingSlider::StaticClass();
	TextLineClassInternal = USettingTextLine::StaticClass();
	UserInputClassInternal = USettingUserInput::StaticClass();

	UserInputThemeDataInternal.Size = FVector2D(128.f);
	UserInputThemeDataInternal.Margin = 0.25f;
	UserInputThemeDataInternal.Padding = FMargin(FVector2D(5.f, 2.5f));
}
