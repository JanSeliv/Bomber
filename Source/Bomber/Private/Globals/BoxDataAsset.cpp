// Copyright (c) Yevhenii Selivanov

#include "Globals/BoxDataAsset.h"
//---
#include "Globals/DataAssetsContainer.h"

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