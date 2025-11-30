// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrWallDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrWallDataAsset)

// Default constructor
UBmrWallDataAsset::UBmrWallDataAsset()
{
	ActorType = EAT::Wall;
}

// Returns the wall data asset
const UBmrWallDataAsset& UBmrWallDataAsset::Get()
{
	return UBmrDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}
