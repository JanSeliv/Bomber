// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BoxDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(BoxDataAsset)

// Default constructor
UBoxDataAsset::UBoxDataAsset()
{
	ActorTypeInternal = EAT::Box;
}

// Returns the box data asset
const UBoxDataAsset& UBoxDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Box);
	const auto BoxDataAsset = Cast<UBoxDataAsset>(FoundDataAsset);
	checkf(BoxDataAsset, TEXT("The Box Data Asset is not valid"));
	return *BoxDataAsset;
}
