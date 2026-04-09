// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrWallDataAsset.h"

// Bomber
#include "DalSubsystem.h"
#include "DataRegistries/BmrWallRow.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrWallDataAsset)

// Default constructor
UBmrWallDataAsset::UBmrWallDataAsset()
{
	ActorType = EAT::Wall;
	RowType = FBmrWallRow::StaticStruct();
}

// Returns the wall data asset
const UBmrWallDataAsset& UBmrWallDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
