// Copyright (c) Yevhenii Selivanov

#include "MyDataTable/MyDataTable.h"
//---
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h" // ReExportTableAsJSON()
#include "UObject/ObjectSaveContext.h"
#endif // WITH_EDITOR
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyDataTable)

// Default constructor to set RowStruct structure inherit from FMyTableRow
UMyDataTable::UMyDataTable()
{
	RowStruct = FMyTableRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this this row
void FMyTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	if (const UMyDataTable* MyDataTable = Cast<UMyDataTable>(InDataTable))
	{
		const uint8& ThisRowPtr = reinterpret_cast<uint8&>(*this);
		UMyDataTable* DataTable = const_cast<UMyDataTable*>(MyDataTable);
		DataTable->OnThisDataTableChanged(InRowName, ThisRowPtr);
	}
}

// Is called on saving the data table
void UMyDataTable::PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext)
{
	Super::PostSaveRoot(ObjectSaveContext);

	ReexportToJson();
}

// Reexports this table to .json
void UMyDataTable::ReexportToJson()
{
	FEditorUtilsLibrary::ReExportTableAsJSON(this);
}
#endif // WITH_EDITOR
