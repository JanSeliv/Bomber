// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrAIDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrAIDataAsset)

// Returns the AI data asset
const UBmrAIDataAsset& UBmrAIDataAsset::Get()
{
	const UBmrAIDataAsset* AIDataAsset = UBmrDataAssetsContainer::GetAIDataAsset();
	checkf(AIDataAsset, TEXT("The AI Data Asset is not valid"));
	return *AIDataAsset;
}
