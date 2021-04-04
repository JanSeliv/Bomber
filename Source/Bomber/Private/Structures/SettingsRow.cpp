// Copyright 2021 Yevhenii Selivanov.

#include "Structures/SettingsRow.h"

// Empty settings row
const FSettingsRow FSettingsRow::EmptyRow = FSettingsRow();

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
bool FSettingsDataBase::operator==(const FSettingsDataBase& Other) const
{
	return GetTypeHash(*this) == GetTypeHash(Other);
}

//
uint32 GetTypeHash(const FSettingsDataBase& Other)
{
	const uint32 TagHash = GetTypeHash(Other.Tag);
	const uint32 ObjectContextHash = GetTypeHash(Other.ObjectContext);
	const uint32 SetterHash = GetTypeHash(Other.Setter);
	const uint32 GetterHash = GetTypeHash(Other.Getter);
	return HashCombine(HashCombine(HashCombine(TagHash, ObjectContextHash), SetterHash), GetterHash);
}

//
const FSettingsDataBase* FSettingsRow::GetChosenSettingsData() const
{
	switch (SettingsType)
	{
		case ESettingsType::None:
			return nullptr;
		case ESettingsType::Button:
			return &Button;
		case ESettingsType::ButtonsRow:
			return &ButtonsRow;
		case ESettingsType::Checkbox:
			return &Checkbox;
		case ESettingsType::Combobox:
			return &Combobox;
		case ESettingsType::Slider:
			return &Slider;
		case ESettingsType::TextSimple:
			return &TextSimple;
		case ESettingsType::TextInput:
			return &TextInput;
		default:
			return nullptr;
	}
}

//
bool FSettingsRow::operator==(const FSettingsRow& Other) const
{
	return GetTypeHash(*this) == GetTypeHash(Other);
}

//
uint32 GetTypeHash(const FSettingsRow& Other)
{
	if (const FSettingsDataBase* OtherRowPtr = Other.GetChosenSettingsData())
	{
		return GetTypeHash(*OtherRowPtr);
	}
	return 0;
}
