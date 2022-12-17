// Copyright (c) Yevhenii Selivanov

#include "Globals/UIDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"

// Returns the UI data asset
const UUIDataAsset& UUIDataAsset::Get()
{
	const UUIDataAsset* UIDataAsset = UDataAssetsContainer::GetUIDataAsset();
	checkf(UIDataAsset, TEXT("The UI Data Asset is not valid"));
	return *UIDataAsset;
}
