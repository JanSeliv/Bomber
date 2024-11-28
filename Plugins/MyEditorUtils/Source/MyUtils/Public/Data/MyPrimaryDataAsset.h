// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"

#include "MyPrimaryDataAsset.generated.h"

/**
 * Is base class for loadable data assets that prevents loading the same data asset multiple times.
 */
UCLASS(Abstract)
class MYUTILS_API UMyPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Loads the data asset by the specified soft pointer or returns the cached one. */
	static const UMyPrimaryDataAsset* GetOrLoadOnce(const TSoftObjectPtr<const UMyPrimaryDataAsset>& DataAssetPtr);

	/** Alternative version of the GetOrLoadOnce method that casts the data asset to the specified type. */
	template <typename T>
	static FORCEINLINE const T* GetOrLoadOnce(const TSoftObjectPtr<const T>& DataAssetPtr) { return Cast<T>(GetOrLoadOnce(TSoftObjectPtr<const UMyPrimaryDataAsset>(DataAssetPtr))); }

	/** Call this method to let garbage collector to unload the data asset. */
	template <typename T>
	static void ResetDataAsset(TSoftObjectPtr<const T>& DataAssetRef);
};

template <typename T>
void UMyPrimaryDataAsset::ResetDataAsset(TSoftObjectPtr<const T>& DataAssetRef)
{
	// Remove from root to allow garbage collection this data asset
	if (const UMyPrimaryDataAsset* DataAsset = DataAssetRef.Get())
	{
		const_cast<UMyPrimaryDataAsset*>(DataAsset)->RemoveFromRoot();
	}

	// Reset the pointer
	if (!DataAssetRef.IsNull())
	{
		DataAssetRef.Reset();
	}
}
