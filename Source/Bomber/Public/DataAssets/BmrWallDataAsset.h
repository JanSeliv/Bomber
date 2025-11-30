// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrWallDataAsset.generated.h"

/**
 * Describes common data for all walls.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrWallDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrWallDataAsset();

	/** Returns the wall data asset. */
	static const UBmrWallDataAsset& Get();
};