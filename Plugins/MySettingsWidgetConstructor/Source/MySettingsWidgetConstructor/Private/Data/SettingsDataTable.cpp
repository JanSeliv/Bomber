// Copyright (c) Yevhenii Selivanov.

#include "Data/SettingsDataTable.h"
//---

// Default constructor to set members as FSettingsRow
USettingsDataTable::USettingsDataTable()
{
	RowStruct = FSettingsRow::StaticStruct();
}

// Returns the table rows
void USettingsDataTable::GenerateSettingsArray(TMap<FName, FSettingsPicker>& OutRows) const
{
	OutRows.Empty();
	OutRows.Reserve(RowMap.Num());
	for (const TTuple<FName, uint8*>& RowIt : RowMap)
	{
		if (const FSettingsRow* FoundRowPtr = reinterpret_cast<const FSettingsRow*>(RowIt.Value))
		{
			const FSettingsPicker& SettingsTableRow = FoundRowPtr->SettingsPicker;
			const FName RowName = RowIt.Key;
			OutRows.Emplace(RowName, SettingsTableRow);
		}
	}
}
