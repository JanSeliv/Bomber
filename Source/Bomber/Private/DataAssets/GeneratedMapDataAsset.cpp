// Copyright (c) Yevhenii Selivanov

#include "DataAssets/GeneratedMapDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GeneratedMapDataAsset)

// Returns the generated map data asset
const UGeneratedMapDataAsset& UGeneratedMapDataAsset::Get()
{
	const UGeneratedMapDataAsset* GeneratedMapDataAsset = UDataAssetsContainer::GetLevelsDataAsset();
	checkf(GeneratedMapDataAsset, TEXT("The Generated Map Data Asset is not valid"))
	return *GeneratedMapDataAsset;
}
