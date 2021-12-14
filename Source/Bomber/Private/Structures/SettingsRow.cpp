// Copyright 2021 Yevhenii Selivanov.

#include "Structures/SettingsRow.h"

// Empty settings row
const FSettingsPicker FSettingsPicker::Empty = FSettingsPicker();

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
