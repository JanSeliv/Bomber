// Copyright (c) Yevhenii Selivanov

#include "MyDataTable/SWCMyDataTable.h"
//---
#if WITH_EDITOR
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#endif // WITH_EDITOR

#if WITH_EDITOR
// Called on every change in this this row
void FSWCMyTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	if (!GEditor && !GEditor->IsPlaySessionInProgress()) // Is Editor not PIE world
	{
		return;
	}

	if (const USWCMyDataTable* SWCMyDataTable = Cast<USWCMyDataTable>(InDataTable))
	{
		const uint8& ThisRowPtr = reinterpret_cast<uint8&>(*this);
		USWCMyDataTable* DataTable = const_cast<USWCMyDataTable*>(SWCMyDataTable);
		DataTable->OnThisDataTableChanged(InRowName, ThisRowPtr);
	}
}
#endif // WITH_EDITOR

// Default constructor to set RowStruct structure inherit from FSWCMyTableRow
USWCMyDataTable::USWCMyDataTable()
{
	RowStruct = FSWCMyTableRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this data table to reexport .json
void USWCMyDataTable::OnThisDataTableChanged(FName RowName, const uint8& RowData)
{
	if (!AssetImportData)
	{
		return;
	}

	const FString CurrentFilename = AssetImportData->GetFirstFilename();
	if (!CurrentFilename.IsEmpty())
	{
		const FString TableAsJSON = GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs);
		FFileHelper::SaveStringToFile(TableAsJSON, *CurrentFilename);
	}
}
#endif // WITH_EDITOR
