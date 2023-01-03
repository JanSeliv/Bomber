// Copyright (c) Yevhenii Selivanov.

#include "Data/SettingsDataTable.h"
//---
#if WITH_EDITOR
#include "DataTableEditorUtils.h" // FDataTableEditorUtils::RenameRow
#endif

// Default constructor to set members as FSettingsRow
USettingsDataTable::USettingsDataTable()
{
	RowStruct = FSettingsRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this data table to automatic set the key name by specified setting tag
void USettingsDataTable::OnThisDataTableChanged(FName RowKey, const uint8& RowData)
{
	// Set row name by specified tag
	const FSettingsRow& Row = reinterpret_cast<const FSettingsRow&>(RowData);
	const FName RowValueTag = Row.SettingsPicker.PrimaryData.Tag.GetTagName();
	if (!RowValueTag.IsNone() // Tag is not empty
		&& RowKey != RowValueTag // New tag name
		&& !RowMap.Contains(RowValueTag)) // Unique key
	{
		FDataTableEditorUtils::RenameRow(this, RowKey, RowValueTag);
	}

	// Export to .json
	Super::OnThisDataTableChanged(RowKey, RowData);
}
#endif // WITH_EDITOR
