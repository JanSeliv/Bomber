// Copyright (c) Yevhenii Selivanov

#include "Globals/DataAssetsContainer.h"
//---
#include "Globals/LevelActorDataAsset.h"
#include "UtilityLibraries/SingletonLibrary.h"

// Returns the data assets container
const UDataAssetsContainer& UDataAssetsContainer::Get()
{
	const UDataAssetsContainer* FoundDataContainer = USingletonLibrary::GetDataAssetsContainer();
	checkf(FoundDataContainer, TEXT("The Data Asset Container is not valid"));
	return *FoundDataContainer;
}

// Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset
const ULevelActorDataAsset* UDataAssetsContainer::GetDataAssetByActorClass(const TSubclassOf<AActor>& ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	const TArray<TObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const ULevelActorDataAsset* DataAssetIt : ActorsDataAssets)
	{
		const UClass* ActorClassIt = DataAssetIt ? DataAssetIt->GetActorClass() : nullptr;
		if (ActorClassIt
		    && ActorClassIt->IsChildOf(ActorClass))
		{
			return DataAssetIt;
		}
	}
	return nullptr;
}

// Iterate ActorsDataAssets array and returns the found Data Assets of level actors by specified types.
void UDataAssetsContainer::GetDataAssetsByActorTypes(TArray<ULevelActorDataAsset*>& OutDataAssets, int32 ActorsTypesBitmask)
{
	const TArray<TObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (ULevelActorDataAsset* DataAssetIt : ActorsDataAssets)
	{
		if (DataAssetIt
		    && USingletonLibrary::BitwiseActorTypes(ActorsTypesBitmask, TO_FLAG(DataAssetIt->GetActorType())))
		{
			OutDataAssets.Emplace(DataAssetIt);
		}
	}
}

// Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type
const ULevelActorDataAsset* UDataAssetsContainer::GetDataAssetByActorType(EActorType ActorType)
{
	TArray<ULevelActorDataAsset*> FoundDataAssets;
	GetDataAssetsByActorTypes(FoundDataAssets, TO_FLAG(ActorType));
	return FoundDataAssets.IsValidIndex(0) ? FoundDataAssets[0] : nullptr;
}

// Iterate ActorsDataAssets array and returns the found actor class by specified actor type
UClass* UDataAssetsContainer::GetActorClassByType(EActorType ActorType)
{
	const ULevelActorDataAsset* FoundDataAsset = GetDataAssetByActorType(ActorType);
	return FoundDataAsset ? FoundDataAsset->GetActorClass() : nullptr;
}
