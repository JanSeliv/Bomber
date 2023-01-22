// Copyright (c) Yevhenii Selivanov

#include "DataAssets/WallDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"

// Default constructor
UWallDataAsset::UWallDataAsset()
{
	ActorTypeInternal = EAT::Wall;
}

// Returns the wall data asset
const UWallDataAsset& UWallDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Wall);
	const UWallDataAsset* WallDataAsset = Cast<UWallDataAsset>(FoundDataAsset);
	checkf(WallDataAsset, TEXT("The Wall Data Asset is not valid"));
	return *WallDataAsset;
}