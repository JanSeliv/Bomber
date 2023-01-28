// Copyright (c) Yevhenii Selivanov

#include "Data/SettingTag.h"
//---
#include "GameplayTags/Classes/GameplayTagsManager.h"

// Setting gag that contains nothing by default
const FSettingTag FSettingTag::EmptySettingTag = EmptyTag;

// The global of Setting tag categories
FGlobalSettingTags FGlobalSettingTags::GSettingTags;

// Automatically adds native setting tags at startup
void FGlobalSettingTags::AddTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	ButtonSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.Button"));
	CheckboxSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.Checkbox"));
	ComboboxSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.Combobox"));
	ScrollboxSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.Scrollbox"));
	SliderSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.Slider"));
	TextLineSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.TextLine"));
	UserInputSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.UserInput"));
	CustomWidgetSettingTag = Manager.AddNativeGameplayTag(TEXT("Settings.CustomWidget"));
}
