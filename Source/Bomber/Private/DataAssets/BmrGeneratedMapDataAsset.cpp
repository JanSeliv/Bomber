// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrGeneratedMapDataAsset.h"

// Bomber
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGeneratedMapDataAsset)

// Returns the generated map data asset
const UBmrGeneratedMapDataAsset& UBmrGeneratedMapDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
