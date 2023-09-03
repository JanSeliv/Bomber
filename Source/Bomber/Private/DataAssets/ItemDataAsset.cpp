// Copyright (c) Yevhenii Selivanov

#include "DataAssets/ItemDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDataAsset)

// Default constructor
UItemDataAsset::UItemDataAsset()
{
	ActorTypeInternal = EAT::Item;
	RowClassInternal = UItemRow::StaticClass();
}

// Returns the item data asset
const UItemDataAsset& UItemDataAsset::Get()
{
	return UDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}

// Return row by specified item type
const UItemRow* UItemDataAsset::GetRowByItemType(EItemType ItemType, ELevelType LevelType) const
{
	TArray<ULevelActorRow*> OutRows;
	GetRowsByLevelType(OutRows, TO_FLAG(LevelType));
	const ULevelActorRow* const* FoundRowPtr = OutRows.FindByPredicate([ItemType](const ULevelActorRow* RowIt)
	{
		const UItemRow* ItemRow = Cast<UItemRow>(RowIt);
		return ItemRow && ItemRow->ItemType == ItemType;
	});
	return FoundRowPtr ? Cast<UItemRow>(*FoundRowPtr) : nullptr;
}
