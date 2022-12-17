// Copyright (c) Yevhenii Selivanov

#include "Globals/GameStateDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"

// Returns the Game State data asset
const UGameStateDataAsset& UGameStateDataAsset::Get()
{
	const UGameStateDataAsset* GameStateDataAsset = UDataAssetsContainer::GetGameStateDataAsset();
	checkf(GameStateDataAsset, TEXT("The Game State Data Asset is not valid"));
	return *GameStateDataAsset;
}