// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrGameStateDataAsset.h"

// Bomber
#include "DalSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameStateDataAsset)

// Returns the Game State data asset
const UBmrGameStateDataAsset& UBmrGameStateDataAsset::Get()
{
	return UDalSubsystem::GetDataAssetChecked<ThisClass>();
}
