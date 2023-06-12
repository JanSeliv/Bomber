// Copyright (c) Yevhenii Selivanov

#include "DataAssets/AIDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(AIDataAsset)

// Returns the AI data asset
const UAIDataAsset& UAIDataAsset::Get()
{
	const UAIDataAsset* AIDataAsset = UDataAssetsContainer::GetAIDataAsset();
	checkf(AIDataAsset, TEXT("The AI Data Asset is not valid"));
	return *AIDataAsset;
}
