// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrPowerupDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "DataRegistries/BmrPowerupRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupDataAsset)

// Default constructor
UBmrPowerupDataAsset::UBmrPowerupDataAsset()
{
	ActorType = EAT::Powerup;
	RowType = FBmrPowerupRow::StaticStruct();
}

// Returns the powerup data asset
const UBmrPowerupDataAsset& UBmrPowerupDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
