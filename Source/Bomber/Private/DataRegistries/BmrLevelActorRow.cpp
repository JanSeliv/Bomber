// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrLevelActorRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrLevelActorRow)

// Returns cached level actor row by runtime struct type and row name, cast to base class
const FBmrLevelActorRow* FBmrLevelActorRow::FindRowByName(const UScriptStruct* RowType, FName RowName)
{
	return FDalRegistryRow::GetTypedRow<FBmrLevelActorRow>(FDalRegistryRow::GetRowByName(RowType, RowName));
}

// Finds first row matching the given level type with ELT::Max fallback, using runtime struct type
const FBmrLevelActorRow* FBmrLevelActorRow::FindRowByLevelType(const UScriptStruct* RowType, EBmrLevelType LevelType)
{
	const FBmrLevelActorRow* FoundRow = nullptr;
	FDalRegistryRow::ForEachRow(RowType, [&FoundRow, LevelType](const uint8* ItemData)
	{
		if (!FoundRow)
		{
			const FBmrLevelActorRow* Row = FDalRegistryRow::GetTypedRow<FBmrLevelActorRow>(ItemData);
			if (Row->LevelType == LevelType
			    || Row->LevelType == ELT::Max)
			{
				FoundRow = Row;
			}
		}
	});
	return FoundRow;
}

// Iterates all Data Registries whose ItemStruct inherits FBmrLevelActorRow
void FBmrLevelActorRow::ForEachLevelActorRegistry(const TFunctionRef<void(UDataRegistry* Registry, const UScriptStruct* ItemStruct)>& Callback)
{
	FDalRegistryRow::ForEachRegistry(StaticStruct(), Callback);
}
