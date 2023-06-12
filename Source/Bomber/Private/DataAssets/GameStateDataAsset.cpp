// Copyright (c) Yevhenii Selivanov

#include "DataAssets/GameStateDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameStateDataAsset)

// Returns the Game State data asset
const UGameStateDataAsset& UGameStateDataAsset::Get()
{
	const UGameStateDataAsset* GameStateDataAsset = UDataAssetsContainer::GetGameStateDataAsset();
	checkf(GameStateDataAsset, TEXT("The Game State Data Asset is not valid"));
	return *GameStateDataAsset;
}
