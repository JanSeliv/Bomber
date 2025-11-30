// Copyright (c) Yevhenii Selivanov

#include "DataAssets/BmrBoxDataAsset.h"

// Bomber
#include "DataAssets/BmrDataAssetsContainer.h"
#include "GameFramework/BmrCheatManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBoxDataAsset)

// Default constructor
UBmrBoxDataAsset::UBmrBoxDataAsset()
{
	ActorType = EAT::Box;
}

// Returns the box data asset
const UBmrBoxDataAsset& UBmrBoxDataAsset::Get()
{
	return UBmrDataAssetsContainer::GetLevelActorDataAssetChecked<ThisClass>();
}

// Returns default value from the data asset of the chance to spawn item after box destroying.
int32 UBmrBoxDataAsset::GetPowerupsChance() const
{
#if !UE_BUILD_SHIPPING
	const int32 CheatOverride = UBmrCheatManager::CVarPowerupsChance.GetValueOnAnyThread();
	if (CheatOverride > 0.f)
	{
		return CheatOverride;
	}
#endif // !UE_BUILD_SHIPPING

	return SpawnItemChance;
}