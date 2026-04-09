// Copyright (c) Yevhenii Selivanov

#include "DataRegistries/BmrPlayerSkinRow.h"

// UE
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerSkinRow)

// Gathers all skin row names for the given player tag from Data Registry
void FBmrPlayerSkinRow::GetAllPlayerSkinNames(const FBmrPlayerTag& PlayerTag, TArray<FName>& OutSkinNames)
{
	OutSkinNames.Reset();
	ForEachRowWithName([&OutSkinNames, &PlayerTag](FName RowName, const FBmrPlayerSkinRow& Row)
	{
		if (Row.PlayerTag == PlayerTag)
		{
			OutSkinNames.Add(RowName);
		}
	});
}

// Returns the total number of skin materials for the given player tag from Data Registry
int32 FBmrPlayerSkinRow::GetSkinTexturesNum(const FBmrPlayerTag& PlayerTag)
{
	TArray<FName> SkinNames;
	GetAllPlayerSkinNames(PlayerTag, SkinNames);
	return SkinNames.Num();
}

// Returns the skin row name for the given player tag, picking by PlayerIndex % available skins count
FName FBmrPlayerSkinRow::GetSkinRowName(const FBmrPlayerTag& PlayerTag, int32 PlayerIndex)
{
	TArray<FName> SkinNames;
	GetAllPlayerSkinNames(PlayerTag, SkinNames);
	if (SkinNames.IsEmpty())
	{
		return NAME_None;
	}

	const int32 SkinIdx = PlayerIndex % SkinNames.Num();
	return SkinNames[SkinIdx];
}

// Returns the skin material for the given player tag and skin index from Data Registry, or nullptr
UMaterialInterface* FBmrPlayerSkinRow::GetSkinMaterial(const FBmrPlayerTag& PlayerTag, int32 SkinIndex)
{
	TArray<FName> SkinNames;
	GetAllPlayerSkinNames(PlayerTag, SkinNames);
	const FBmrPlayerSkinRow* SkinRow = SkinNames.IsValidIndex(SkinIndex) ? GetRowByName(SkinNames[SkinIndex]) : nullptr;
	return SkinRow ? SkinRow->Material.Get() : nullptr;
}
