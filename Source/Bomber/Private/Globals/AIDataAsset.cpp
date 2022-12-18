// Copyright (c) Yevhenii Selivanov

#include "Globals/AIDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"

// Returns the AI data asset
const UAIDataAsset& UAIDataAsset::Get()
{
	const UAIDataAsset* AIDataAsset = UDataAssetsContainer::GetAIDataAsset();
	checkf(AIDataAsset, TEXT("The AI Data Asset is not valid"));
	return *AIDataAsset;
}