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
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Item);
	const UItemDataAsset* ItemDataAsset = Cast<UItemDataAsset>(FoundDataAsset);
	checkf(ItemDataAsset, TEXT("The Item Data Asset is not valid"));
	return *ItemDataAsset;
}

// Return row by specified item type
const UItemRow* UItemDataAsset::GetRowByItemType(EItemType ItemType, ELevelType LevelType) const
{
	TArray<ULevelActorRow*> OutRows;
	Get().GetRowsByLevelType(OutRows, TO_FLAG(LevelType));
	const ULevelActorRow* const* FoundRowPtr = OutRows.FindByPredicate([ItemType](const ULevelActorRow* RowIt)
	{
		const auto ItemRow = Cast<UItemRow>(RowIt);
		return ItemRow && ItemRow->ItemType == ItemType;
	});
	return FoundRowPtr ? Cast<UItemRow>(*FoundRowPtr) : nullptr;
}
