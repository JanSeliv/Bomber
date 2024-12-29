// Copyright (c) Yevhenii Selivanov

#include "DataAssets/DataAssetsContainer.h"
//---
#include "Bomber.h"
#include "DataAssets/AIDataAsset.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/GeneratedMapDataAsset.h"
#include "DataAssets/LevelActorDataAsset.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "DataAssets/SoundsDataAsset.h"
#include "DataAssets/UIDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(DataAssetsContainer)

// Returns the Levels Data Asset
const UGeneratedMapDataAsset* UDataAssetsContainer::GetGeneratedMapDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UGeneratedMapDataAsset>(Get().GeneratedMapDataAssetInternal);
}

// Returns the UI Data Asset
const UUIDataAsset* UDataAssetsContainer::GetUIDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UUIDataAsset>(Get().UIDataAssetInternal);
}

// Returns the AI Data Asset
const UAIDataAsset* UDataAssetsContainer::GetAIDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UAIDataAsset>(Get().AIDataAssetInternal);
}

// Returns the Player Input Data Asset
const UPlayerInputDataAsset* UDataAssetsContainer::GetPlayerInputDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UPlayerInputDataAsset>(Get().PlayerInputDataAssetInternal);
}

// Returns the Sounds Data Asset
const USoundsDataAsset* UDataAssetsContainer::GetSoundsDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<USoundsDataAsset>(Get().SoundsDataAssetInternal);
}

// Returns the Game State Data Asset
const UGameStateDataAsset* UDataAssetsContainer::GetGameStateDataAsset()
{
	return UMyPrimaryDataAsset::GetOrLoadOnce<UGameStateDataAsset>(Get().GameStateDataAssetInternal);
}

// Best suits for blueprints to get the data asset by its class since converts the result to the specified class
const ULevelActorDataAsset* UDataAssetsContainer::GetLevelActorDataAsset(TSubclassOf<ULevelActorDataAsset> DataAssetClass)
{
	if (!DataAssetClass)
	{
		return nullptr;
	}

	const UDataAssetsContainer& Container = Get();
	for (const TSoftObjectPtr<const ULevelActorDataAsset>& DataAssetSoftIt : Container.ActorsDataAssetsInternal)
	{
		const ULevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);
		if (DataAssetIt->IsA(DataAssetClass))
		{
			return DataAssetIt;
		}
	}

	return nullptr;
}

// Iterate ActorsDataAssets array and returns the found Level Actor class by specified data asset
const ULevelActorDataAsset* UDataAssetsContainer::GetDataAssetByActorClass(const TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	const UDataAssetsContainer& Container = Get();
	for (const TSoftObjectPtr<const ULevelActorDataAsset>& DataAssetSoftIt : Container.ActorsDataAssetsInternal)
	{
		const ULevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);

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
void UDataAssetsContainer::GetDataAssetsByActorTypes(TArray<const ULevelActorDataAsset*>& OutDataAssets, int32 ActorsTypesBitmask)
{
	const TArray<TSoftObjectPtr<const ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TSoftObjectPtr<const ULevelActorDataAsset>& DataAssetSoftIt : ActorsDataAssets)
	{
		const ULevelActorDataAsset* DataAssetIt = UMyPrimaryDataAsset::GetOrLoadOnce(DataAssetSoftIt);

		if (DataAssetIt
		    && (ActorsTypesBitmask & TO_FLAG(DataAssetIt->GetActorType())) != 0)
		{
			OutDataAssets.Emplace(DataAssetIt);
		}
	}
}

// Iterate ActorsDataAssets array and return the first found Data Assets of level actors by specified type
const ULevelActorDataAsset* UDataAssetsContainer::GetDataAssetByActorType(EActorType ActorType)
{
	TArray<const ULevelActorDataAsset*> FoundDataAssets;
	GetDataAssetsByActorTypes(FoundDataAssets, TO_FLAG(ActorType));
	return FoundDataAssets.IsValidIndex(0) ? FoundDataAssets[0] : nullptr;
}

// Iterate ActorsDataAssets array and returns the found actor class by specified actor type
UClass* UDataAssetsContainer::GetActorClassByType(EActorType ActorType)
{
	const ULevelActorDataAsset* FoundDataAsset = GetDataAssetByActorType(ActorType);
	return FoundDataAsset ? FoundDataAsset->GetActorClass() : nullptr;
}