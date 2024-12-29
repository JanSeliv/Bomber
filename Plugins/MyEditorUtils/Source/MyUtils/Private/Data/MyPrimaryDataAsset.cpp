// Copyright (c) Yevhenii Selivanov

#include "Data/MyPrimaryDataAsset.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyPrimaryDataAsset)

// Loads the data asset by the specified soft pointer or returns the cached one
const UMyPrimaryDataAsset* UMyPrimaryDataAsset::GetOrLoadOnce(const TSoftObjectPtr<const UMyPrimaryDataAsset>& DataAssetPtr)
{
	if (!ensureMsgf(!DataAssetPtr.IsNull(), TEXT("ASSERT: [%i] %hs:\n'DataAssetPtr' likely is not even set in the editor!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	if (const UObject* Asset = DataAssetPtr.Get())
	{
		// Is already loaded, in most cases this will be the case
		return Cast<UMyPrimaryDataAsset>(Asset);
	}

	// Load the asset
	UMyPrimaryDataAsset* LoadedDataAsset = Cast<UMyPrimaryDataAsset>(DataAssetPtr.ToSoftObjectPath().TryLoad());
	if (!ensureMsgf(LoadedDataAsset, TEXT("ASSERT: [%i] %hs:\n'%s' failed to load!"), __LINE__, __FUNCTION__, *DataAssetPtr.ToString()))
	{
		return nullptr;
	}

	// Add to root to prevent garbage collection this data asset, we should enter here only once per data asset
	LoadedDataAsset->AddToRoot();

	return LoadedDataAsset;
}
