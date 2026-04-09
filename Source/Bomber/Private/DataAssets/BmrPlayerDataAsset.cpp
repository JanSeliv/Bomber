// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPlayerDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "DataRegistries/BmrPlayerRow.h"
#include "DataRegistries/BmrPlayerSkinRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerDataAsset)

// Default constructor
UBmrPlayerDataAsset::UBmrPlayerDataAsset()
{
	ActorType = EAT::Player;
	RowType = FBmrPlayerRow::StaticStruct();
}

// Returns the player data asset
const UBmrPlayerDataAsset& UBmrPlayerDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Get nameplate material by index, is used by nameplate meshes
UMaterialInterface* UBmrPlayerDataAsset::GetNameplateMaterial(int32 Index) const
{
	if (NameplateMaterials.IsValidIndex(Index))
	{
		return NameplateMaterials[Index];
	}

	return nullptr;
}

// Returns the Data Registry row name for the given player tag, or NAME_None
FName UBmrPlayerDataAsset::GetRowNameByPlayerTag(const FBmrPlayerTag& PlayerTag)
{
	return FBmrPlayerRow::GetRowNameByPlayerTag(PlayerTag);
}

// Returns the total number of skin materials for the given player tag from FBmrPlayerSkinRow
int32 UBmrPlayerDataAsset::GetSkinTexturesNum(const FBmrPlayerTag& PlayerTag)
{
	return FBmrPlayerSkinRow::GetSkinTexturesNum(PlayerTag);
}

// Returns the skin material for the given player tag and skin index from FBmrPlayerSkinRow, or nullptr
UMaterialInterface* UBmrPlayerDataAsset::GetSkinMaterial(const FBmrPlayerTag& PlayerTag, int32 SkinIndex)
{
	return FBmrPlayerSkinRow::GetSkinMaterial(PlayerTag, SkinIndex);
}

// Returns player row data by mesh from Data Registry, returns empty row data if not found
const FBmrPlayerRow& UBmrPlayerDataAsset::GetRowByMesh(const UStreamableRenderAsset* Mesh)
{
	return FBmrPlayerRow::GetRowByMesh(Mesh);
}