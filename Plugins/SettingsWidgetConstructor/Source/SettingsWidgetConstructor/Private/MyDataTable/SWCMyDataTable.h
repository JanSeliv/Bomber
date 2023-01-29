// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"
//---
#include "SWCMyDataTable.generated.h"

/**
 * Base class for all custom data table row structs to inherit from.
 */
USTRUCT(BlueprintType)
struct FSWCMyTableRow : public FTableRowBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	/** Called on every change in this this row.
	 * @see USWCMyDataTable::OnThisDataTableChanged. */
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
#endif // WITH_EDITOR
};

/**
 * Provides additional in-editor functionality like reexporting .json on data table change.
 * Its RowStruct structure must inherit from FSWCMyTableRow.
 */
UCLASS()
class USWCMyDataTable : public UDataTable
{
	GENERATED_BODY()

public:
	/** Default constructor to set RowStruct structure inherit from FSWCMyTableRow. */
	USWCMyDataTable();

	/** Returns the table rows. */
	template <typename T>
	void GetRows(TMap<FName, T>& OutRows) const;

protected:
#pragma region OnDataTableChange
#if WITH_EDITOR
	friend FSWCMyTableRow;

	/** Called on every change in this data table to reexport .json.
	 * Is created to let child data tables reacts on changes without binding to its delegate,
	 * can't use UDataTable::HandleDataTableChanged() since it is not virtual.
	 * Is in runtime module since FDataTableEditor is private. */
	virtual void OnThisDataTableChanged(FName RowName, const uint8& RowData);
#endif // WITH_EDITOR
#pragma endregion OnDataTableChange
};

/** Returns the table rows. */
template <typename T>
void USWCMyDataTable::GetRows(TMap<FName, T>& OutRows) const
{
	static_assert(TIsDerivedFrom<T, FSWCMyTableRow>::Value, "Type is not derived from FSWCMyTableRow.");
	if (ensureAlwaysMsgf(RowStruct && RowStruct->IsChildOf(T::StaticStruct()), TEXT("ASSERT: 'RowStruct' is not child of specified struct")))
	{
		OutRows.Empty();
		OutRows.Reserve(RowMap.Num());
		for (const TTuple<FName, uint8*>& RowIt : RowMap)
		{
			if (const T* FoundRowPtr = reinterpret_cast<const T*>(RowIt.Value))
			{
				OutRows.Emplace(RowIt.Key, *FoundRowPtr);
			}
		}
	}
}
