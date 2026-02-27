// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPowerupDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupDataAsset)

// Default constructor
UBmrPowerupDataAsset::UBmrPowerupDataAsset()
{
	ActorType = EAT::Powerup;
	RowClass = UBmrPowerupRow::StaticClass();
}

// Returns the powerup data asset
const UBmrPowerupDataAsset& UBmrPowerupDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}

// Return row by specified powerup type
const UBmrPowerupRow* UBmrPowerupDataAsset::GetRowByPowerupTag(FBmrPowerupTag Tag, EBmrLevelType LevelType /* = EBmrLevelType::None*/) const
{
	if (LevelType == EBmrLevelType::None)
	{
		LevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	}

	TArray<UBmrLevelActorRow*> OutRows;
	GetRowsByLevelType(OutRows, TO_FLAG(LevelType));
	const UBmrLevelActorRow* const* FoundRowPtr = OutRows.FindByPredicate([Tag](const UBmrLevelActorRow* RowIt)
	{
		const UBmrPowerupRow* PowerupRow = Cast<UBmrPowerupRow>(RowIt);
		return PowerupRow && PowerupRow->PowerupTag == Tag;
	});
	return FoundRowPtr ? Cast<UBmrPowerupRow>(*FoundRowPtr) : nullptr;
}
