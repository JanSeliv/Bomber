// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrGeneratedMapDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGeneratedMapDataAsset)

// Returns the generated map data asset
const UBmrGeneratedMapDataAsset& UBmrGeneratedMapDataAsset::Get()
{
	const UBmrGeneratedMapDataAsset* GeneratedMapDataAsset = UBmrDataAssetsContainer::GetGeneratedMapDataAsset();
	checkf(GeneratedMapDataAsset, TEXT("The Generated Map Data Asset is not valid")) return *GeneratedMapDataAsset;
}
