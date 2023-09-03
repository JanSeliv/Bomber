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
const UGeneratedMapDataAsset* UDataAssetsContainer::GetLevelsDataAsset()
{
	const UGeneratedMapDataAsset* LevelsDataAsset = Get().LevelsDataAssetInternal.LoadSynchronous();
	checkf(LevelsDataAsset, TEXT("%s: 'LevelsDataAsset' is not loaded"), *FString(__FUNCTION__));
	return LevelsDataAsset;
}

// Returns the UI Data Asset
const UUIDataAsset* UDataAssetsContainer::GetUIDataAsset()
{
	const UUIDataAsset* UIDataAsset = Get().UIDataAssetInternal.LoadSynchronous();
	checkf(UIDataAsset, TEXT("%s: 'UIDataAsset' is not loaded"), *FString(__FUNCTION__));
	return UIDataAsset;
}

// Returns the AI Data Asset
const UAIDataAsset* UDataAssetsContainer::GetAIDataAsset()
{
	const UAIDataAsset* AIDataAsset = Get().AIDataAssetInternal.LoadSynchronous();
	checkf(AIDataAsset, TEXT("%s: 'AIDataAsset' is not loaded"), *FString(__FUNCTION__));
	return AIDataAsset;
}

// Returns the Player Input Data Asset
const UPlayerInputDataAsset* UDataAssetsContainer::GetPlayerInputDataAsset()
{
	const UPlayerInputDataAsset* PlayerInputDataAsset = Get().PlayerInputDataAssetInternal.LoadSynchronous();
	checkf(PlayerInputDataAsset, TEXT("%s: 'PlayerInputDataAsset' is not loaded"), *FString(__FUNCTION__));
	return PlayerInputDataAsset;
}

// Returns the Sounds Data Asset
const USoundsDataAsset* UDataAssetsContainer::GetSoundsDataAsset()
{
	const USoundsDataAsset* SoundsDataAsset = Get().SoundsDataAssetInternal.LoadSynchronous();
	checkf(SoundsDataAsset, TEXT("%s: 'SoundsDataAsset' is not loaded"), *FString(__FUNCTION__));
	return SoundsDataAsset;
}

// Returns the Game State Data Asset
const UGameStateDataAsset* UDataAssetsContainer::GetGameStateDataAsset()
{
	const UGameStateDataAsset* GameStateDataAsset = Get().GameStateDataAssetInternal.LoadSynchronous();
	checkf(GameStateDataAsset, TEXT("%s: 'GameStateDataAsset' is not loaded"), *FString(__FUNCTION__));
	return GameStateDataAsset;
}

// Best suits for blueprints to get the data asset by its class since converts the result to the specified class
const ULevelActorDataAsset* UDataAssetsContainer::GetLevelActorDataAsset(TSubclassOf<ULevelActorDataAsset> DataAssetClass)
{
	if (!DataAssetClass)
	{
		return nullptr;
	}

	const TArray<TSoftObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TSoftObjectPtr<ULevelActorDataAsset>& DataAssetSoftIt : ActorsDataAssets)
	{
		const ULevelActorDataAsset* DataAssetIt = DataAssetSoftIt.LoadSynchronous();
		checkf(DataAssetIt, TEXT("%s: 'DataAssetIt' is not loaded"), *FString(__FUNCTION__));
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

	const TArray<TSoftObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TSoftObjectPtr<ULevelActorDataAsset>& DataAssetSoftIt : ActorsDataAssets)
	{
		const ULevelActorDataAsset* DataAssetIt = DataAssetSoftIt.LoadSynchronous();
		checkf(DataAssetIt, TEXT("%s: 'DataAssetIt' is not loaded"), *FString(__FUNCTION__));

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
	const TArray<TSoftObjectPtr<ULevelActorDataAsset>>& ActorsDataAssets = Get().ActorsDataAssetsInternal;
	for (const TSoftObjectPtr<ULevelActorDataAsset>& DataAssetSoftIt : ActorsDataAssets)
	{
		ULevelActorDataAsset* DataAssetIt = DataAssetSoftIt.LoadSynchronous();
		checkf(DataAssetIt, TEXT("%s: 'DataAssetIt' is not loaded"), *FString(__FUNCTION__));

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
