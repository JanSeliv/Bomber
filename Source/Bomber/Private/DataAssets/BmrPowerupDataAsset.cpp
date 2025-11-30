// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPowerupDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupDataAsset)

// Default constructor
UBmrPowerupDataAsset::UBmrPowerupDataAsset()
{
	ActorType = EAT::Item;
	RowClass = UBmrPowerupRow::StaticClass();
}

// Returns the item data asset
const UBmrPowerupDataAsset& UBmrPowerupDataAsset::Get()
{
	return UBmrDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}

// Return row by specified item type
const UBmrPowerupRow* UBmrPowerupDataAsset::GetRowByItemType(FBmrPowerupTag ItemType, EBmrLevelType LevelType) const
{
	TArray<UBmrLevelActorRow*> OutRows;
	GetRowsByLevelType(OutRows, TO_FLAG(LevelType));
	const UBmrLevelActorRow* const* FoundRowPtr = OutRows.FindByPredicate([ItemType](const UBmrLevelActorRow* RowIt)
	{
		const UBmrPowerupRow* ItemRow = Cast<UBmrPowerupRow>(RowIt);
		return ItemRow && ItemRow->ItemType == ItemType;
	});
	return FoundRowPtr ? Cast<UBmrPowerupRow>(*FoundRowPtr) : nullptr;
}
