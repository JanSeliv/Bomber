// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BoxDataAsset.h"
//---
#include "DataAssets/DataAssetsContainer.h"
#include "GameFramework/MyCheatManager.h"
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
	return UDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}

// Returns default value from the data asset of the chance to spawn item after box destroying.
int32 UBoxDataAsset::GetPowerupsChance() const
{
#if !UE_BUILD_SHIPPING
	const int32 CheatOverride = UMyCheatManager::CVarPowerupsChance.GetValueOnAnyThread();
	if (CheatOverride > 0.f)
	{
		return CheatOverride;
	}
#endif //!UE_BUILD_SHIPPING

	return SpawnItemChanceInternal;
}