// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrPlayerRow.h"

// UE
#include "Engine/StreamableRenderAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerRow)

// Returns player row data by player tag from Data Registry, or nullptr
const FBmrPlayerRow* FBmrPlayerRow::GetRowByPlayerTag(const FBmrPlayerTag& PlayerTag)
{
	return GetRowByPredicate([&PlayerTag](const FBmrPlayerRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});
}

// Returns the Data Registry row name for the given player tag, or NAME_None
FName FBmrPlayerRow::GetRowNameByPlayerTag(const FBmrPlayerTag& PlayerTag)
{
	return GetRowNameByPredicate([&PlayerTag](const FBmrPlayerRow& Row)
	{
		return Row.PlayerTag == PlayerTag;
	});
}

// Returns player row data by mesh from Data Registry, returns empty row data if not found
const FBmrPlayerRow& FBmrPlayerRow::GetRowByMesh(const UStreamableRenderAsset* Mesh)
{
	static const FBmrPlayerRow EmptyRowData;
	if (!Mesh)
	{
		return EmptyRowData;
	}

	const FSoftObjectPath MeshPath(Mesh);
	const FBmrPlayerRow* FoundRow = GetRowByPredicate([&MeshPath](const FBmrPlayerRow& Row)
	{
		return Row.Mesh.ToSoftObjectPath() == MeshPath;
	});
	return FoundRow ? *FoundRow : EmptyRowData;
}
