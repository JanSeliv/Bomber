// Copyright (c) Yevhenii Selivanov.

#include "Structures/SettingsRow.h"
//---
#include "GameplayTags/Classes/GameplayTagsManager.h"

// Empty settings tag
const FSettingTag FSettingTag::EmptySettingTag = EmptyTag;

// Empty settings primary row
const FSettingsPrimary FSettingsPrimary::EmptyPrimary = FSettingsPrimary();

// Empty settings row
const FSettingsPicker FSettingsPicker::Empty = FSettingsPicker();

// The global of Setting tag categories
FGlobalSettingTags FGlobalSettingTags::GSettingTags;

// Empty setting function data
const FSettingFunctionPicker FSettingFunctionPicker::EmptySettingFunction = FSettingFunctionPicker();

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

// Default constructor to set default values
FButtonThemeData::FButtonThemeData()
{
	Size = FVector2D(128.f);
	Margin = 0.25f;
}

// Default constructor to set default values
FCheckboxThemeData::FCheckboxThemeData()
{
	Size = FVector2D(24.f);
	DrawAs = ESlateBrushDrawType::Image;
}

// Default constructor to set default values
FComboboxThemeData::FComboboxThemeData()
{
	PressedPadding = FMargin(4.f, 6.5f, 2.f, 9.f);

	Arrow.Size = FVector2D(32.f);
	Arrow.DrawAs = ESlateBrushDrawType::Image;

	Border.Size = FVector2D(128.f);
	Border.Margin = 0.45f;
	Border.Padding = 5.f;

	Size = FVector2D(128.f);
	Margin = 0.25f;
	Padding = FMargin(4.f, 6.5f, 2.f, 9.f);
}

// Default constructor to set default values
FSliderThemeData::FSliderThemeData()
{
	Thumb.Size = FVector2D(12.f, 24.f);
	Thumb.DrawAs = ESlateBrushDrawType::Image;

	Size = FVector2D(128.f, 32.f);
}

// Default constructor to set default values
FMiscThemeData::FMiscThemeData()
{
	static const FLinearColor BlackOlive(FVector(0.046665f));
	ThemeColorNormal = BlackOlive;

	static const FLinearColor Nickel(FVector(0.168269f));
	ThemeColorHover = Nickel;
	ThemeColorExtra = Nickel;

	TooltipBackground.Size = FVector2D(128.f, 64.f);
	TooltipBackground.Margin = 0.2f;

	WindowBackground.Size = FVector2D(128.f, 64.f);
	WindowBackground.Margin = 0.3f;

	MenuBorderData.Margin = 0.45f;
}

// Compares for equality
bool FSettingsPrimary::operator==(const FSettingsPrimary& Other) const
{
	return GetTypeHash(*this) == GetTypeHash(Other);
}

// Creates a hash value
uint32 GetTypeHash(const FSettingsPrimary& Other)
{
	const uint32 TagHash = GetTypeHash(Other.Tag);
	const uint32 ObjectContextHash = GetTypeHash(Other.StaticContext);
	const uint32 SetterHash = GetTypeHash(Other.Setter);
	const uint32 GetterHash = GetTypeHash(Other.Getter);
	const uint32 CaptionHash = GetTypeHash(Other.Caption.ToString());
	const uint32 TooltipHash = GetTypeHash(Other.Tooltip.ToString());
	const uint32 PaddingLeftHash = GetTypeHash(Other.Padding.Left);
	const uint32 PaddingTopHash = GetTypeHash(Other.Padding.Top);
	const uint32 PaddingRightHash = GetTypeHash(Other.Padding.Right);
	const uint32 PaddingBottomHash = GetTypeHash(Other.Padding.Bottom);
	const uint32 LineHeightHash = GetTypeHash(Other.LineHeight);
	const uint32 StartOnNextColumnHash = GetTypeHash(Other.bStartOnNextColumn);
	const uint32 SettingsToUpdateHash = GetTypeHash(Other.SettingsToUpdate.ToStringSimple());
	return HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(
		TagHash, ObjectContextHash), SetterHash), GetterHash), CaptionHash), TooltipHash), PaddingLeftHash), PaddingTopHash), PaddingRightHash), PaddingBottomHash), LineHeightHash), StartOnNextColumnHash), SettingsToUpdateHash);
}

// Returns the pointer to one of the chosen in-game type
const FSettingsDataBase* FSettingsPicker::GetChosenSettingsData() const
{
	const FSettingsDataBase* FoundSetting = nullptr;
	if (!SettingsType.IsNone())
	{
		static const UScriptStruct* const& SettingsPickerStruct = StaticStruct();
		const FProperty* FoundProperty = SettingsPickerStruct ? SettingsPickerStruct->FindPropertyByName(SettingsType) : nullptr;
		const FStructProperty* FoundStructProperty = CastField<FStructProperty>(FoundProperty);
		FoundSetting = FoundStructProperty ? FoundStructProperty->ContainerPtrToValuePtr<FSettingsDataBase>(this, 0) : nullptr;
	}
	return FoundSetting;
}

// Compares for equality
bool FSettingsPicker::operator==(const FSettingsPicker& Other) const
{
	return GetChosenSettingsData() == Other.GetChosenSettingsData()
		&& GetTypeHash(*this) == GetTypeHash(Other);
}

// Creates a hash value
uint32 GetTypeHash(const FSettingsPicker& Other)
{
	return GetTypeHash(Other.PrimaryData);
}
