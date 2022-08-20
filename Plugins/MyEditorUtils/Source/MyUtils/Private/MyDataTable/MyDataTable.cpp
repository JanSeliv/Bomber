// Copyright (c) Yevhenii Selivanov

#include "MyDataTable//MyDataTable.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h" // ReExportTableAsJSON()
#endif //WITH_EDITOR

#if WITH_EDITOR
// Called on every change in this this row
void FMyTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	if (const UMyDataTable* MyDataTable = Cast<UMyDataTable>(InDataTable))
	{
		const uint8& ThisRowPtr = reinterpret_cast<uint8&>(*this);
		UMyDataTable* DataTable = const_cast<UMyDataTable*>(MyDataTable);
		DataTable->OnThisDataTableChanged(InRowName, ThisRowPtr);
	}
}
#endif // WITH_EDITOR

// Default constructor to set RowStruct structure inherit from FMyTableRow
UMyDataTable::UMyDataTable()
{
	RowStruct = FMyTableRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this data table to reexport .json
void UMyDataTable::OnThisDataTableChanged(FName RowName, const uint8& RowData)
{
	UEditorUtilsLibrary::ReExportTableAsJSON(this);
}
#endif // WITH_EDITOR
