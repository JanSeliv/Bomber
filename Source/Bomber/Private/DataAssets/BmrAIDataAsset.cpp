// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrAIDataAsset.h"

// Bomber
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrAIDataAsset)

// Returns the AI data asset
const UBmrAIDataAsset& UBmrAIDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
