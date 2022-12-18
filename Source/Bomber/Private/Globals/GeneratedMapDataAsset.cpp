﻿// Copyright (c) Yevhenii Selivanov

#include "Globals/GeneratedMapDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"

// Returns the generated map data asset
const UGeneratedMapDataAsset& UGeneratedMapDataAsset::Get()
{
	const UGeneratedMapDataAsset* GeneratedMapDataAsset = UDataAssetsContainer::GetLevelsDataAsset();
	checkf(GeneratedMapDataAsset, TEXT("The Generated Map Data Asset is not valid"))
	return *GeneratedMapDataAsset;
}