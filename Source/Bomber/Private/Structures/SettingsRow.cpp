// Copyright 2021 Yevhenii Selivanov.

#include "Structures/SettingsRow.h"

// Empty settings function
const FSettingsFunction FSettingsFunction::Empty = FSettingsFunction();

// Empty settings row
const FSettingsPicker FSettingsPicker::Empty = FSettingsPicker();

//
bool FSettingsFunction::operator==(const FSettingsFunction& Other) const
{
	return GetTypeHash(*this) == GetTypeHash(Other);
}

//
uint32 GetTypeHash(const FSettingsFunction& Other)
{
	const uint32 FunctionClassHash = GetTypeHash(Other.FunctionClass);
	const uint32 FunctionNameHash = GetTypeHash(Other.FunctionName);
	return HashCombine(FunctionClassHash, FunctionNameHash);
}

//
const FSettingsDataBase* FSettingsPicker::GetChosenSettingsData() const
{
	if (SettingsType.IsNone())
	{
		return nullptr;
	}

	const FStructProperty* FoundProperty = CastField<FStructProperty>(StaticStruct()->FindPropertyByName(SettingsType));
	return FoundProperty ? FoundProperty->ContainerPtrToValuePtr<FSettingsDataBase>(this, 0) : nullptr;
}

//
bool FSettingsPicker::operator==(const FSettingsPicker& Other) const
{
	return GetChosenSettingsData() == Other.GetChosenSettingsData()
	       && GetTypeHash(*this) == GetTypeHash(Other);
}

//
uint32 GetTypeHash(const FSettingsPicker& Other)
{
	const uint32 TagHash = GetTypeHash(Other.Tag);
	const uint32 ObjectContextHash = GetTypeHash(Other.StaticContext);
	const uint32 SetterHash = GetTypeHash(Other.Setter);
	const uint32 GetterHash = GetTypeHash(Other.Getter);
	return HashCombine(HashCombine(HashCombine(TagHash, ObjectContextHash), SetterHash), GetterHash);
}
