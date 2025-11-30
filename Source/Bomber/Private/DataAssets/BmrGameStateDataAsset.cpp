// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrGameStateDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameStateDataAsset)

// Returns the Game State data asset
const UBmrGameStateDataAsset& UBmrGameStateDataAsset::Get()
{
	const UBmrGameStateDataAsset* GameStateDataAsset = UBmrDataAssetsContainer::GetGameStateDataAsset();
	checkf(GameStateDataAsset, TEXT("The Game State Data Asset is not valid"));
	return *GameStateDataAsset;
}
