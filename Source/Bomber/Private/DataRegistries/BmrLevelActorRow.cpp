// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrLevelActorRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrLevelActorRow)

// Returns cached level actor row by runtime struct type and row name, cast to base class
const FBmrLevelActorRow* FBmrLevelActorRow::FindRowByName(const UScriptStruct* RowType, FName RowName)
{
	return FDalRegistryRow::GetTypedRow<FBmrLevelActorRow>(FDalRegistryRow::GetRowByName(RowType, RowName));
}

// Returns the first cached level actor row for the given runtime struct type, or nullptr
const FBmrLevelActorRow* FBmrLevelActorRow::FindFirstRow(const UScriptStruct* RowType)
{
	return FDalRegistryRow::GetTypedRow<FBmrLevelActorRow>(FDalRegistryRow::GetFirstRow(RowType));
}
