// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrDataAssetsContainer.h"

// Bomber
#include "Bomber.h"
#include "DataAssets/BmrAIDataAsset.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataAssets/BmrGeneratedMapDataAsset.h"
#include "DataAssets/BmrLevelActorDataAsset.h"
#include "DataAssets/BmrPlayerInputDataAsset.h"
#include "DataAssets/BmrSoundsDataAsset.h"
#include "DataAssets/BmrUIDataAsset.h"

// UE
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrDataAssetsContainer)

// Returns the Levels Data Asset
const UBmrGeneratedMapDataAsset* UBmrDataAssetsContainer::GetGeneratedMapDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrGeneratedMapDataAsset>(Get().GeneratedMapDataAsset);
}

// Returns the UI Data Asset
const UBmrUIDataAsset* UBmrDataAssetsContainer::GetUIDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrUIDataAsset>(Get().UIDataAsset);
}

// Returns the AI Data Asset
const UBmrAIDataAsset* UBmrDataAssetsContainer::GetAIDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrAIDataAsset>(Get().AIDataAsset);
}

// Returns the Player Input Data Asset
const UBmrPlayerInputDataAsset* UBmrDataAssetsContainer::GetPlayerInputDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrPlayerInputDataAsset>(Get().PlayerInputDataAsset);
}

// Returns the Sounds Data Asset
const UBmrSoundsDataAsset* UBmrDataAssetsContainer::GetSoundsDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrSoundsDataAsset>(Get().SoundsDataAsset);
}

// Returns the Game State Data Asset
const UBmrGameStateDataAsset* UBmrDataAssetsContainer::GetGameStateDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UBmrGameStateDataAsset>(Get().GameStateDataAsset);
}

// Best suits for blueprints to get the data asset by its class since converts the result to the specified class
const UBmrLevelActorDataAsset* UBmrDataAssetsContainer::GetLevelActorDataAsset(TSubclassOf<UBmrLevelActorDataAsset> DataAssetClass)
{
	if (!DataAssetClass)
	{
		return nullptr;
	}

	const UBmrDataAssetsContainer& Container = Get();
	for (const TSoftObjectPtr<const UBmrLevelActorDataAsset>& DataAssetSoftIt : Container.ActorsDataAssets)
	{
		const UBmrLevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);
		if (DataAssetIt->IsA(DataAssetClass))
		{
			return DataAssetIt;
		}
	}

	return nullptr;
}

// Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset
const UBmrLevelActorDataAsset* UBmrDataAssetsContainer::GetDataAssetByActorClass(const TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	const UBmrDataAssetsContainer& Container = Get();
	for (const TSoftObjectPtr<const UBmrLevelActorDataAsset>& DataAssetSoftIt : Container.ActorsDataAssets)
	{
		const UBmrLevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);

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
void UBmrDataAssetsContainer::GetDataAssetsByActorTypes(TArray<const UBmrLevelActorDataAsset*>& OutDataAssets, int32 ActorsTypesBitmask)
{
	const TArray<TSoftObjectPtr<const UBmrLevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssets;
	for (const TSoftObjectPtr<const UBmrLevelActorDataAsset>& DataAssetSoftIt : ActorsDataAssets)
	{
		const UBmrLevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);

		if (DataAssetIt
		    && (ActorsTypesBitmask & TO_FLAG(DataAssetIt->GetActorType())) != 0)
		{
			OutDataAssets.Emplace(DataAssetIt);
		}
	}
}

// Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type
const UBmrLevelActorDataAsset* UBmrDataAssetsContainer::GetDataAssetByActorType(EBmrActorType ActorType)
{
	TArray<const UBmrLevelActorDataAsset*> FoundDataAssets;
	GetDataAssetsByActorTypes(FoundDataAssets, TO_FLAG(ActorType));
	return FoundDataAssets.IsValidIndex(0) ? FoundDataAssets[0] : nullptr;
}

// Iterate ActorsDataAssets array and returns the found actor class by specified actor type
UClass* UBmrDataAssetsContainer::GetActorClassByType(EBmrActorType ActorType)
{
	const UBmrLevelActorDataAsset* FoundDataAsset = GetDataAssetByActorType(ActorType);
	return FoundDataAsset ? FoundDataAsset->GetActorClass() : nullptr;
}