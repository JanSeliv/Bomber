// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrWallDataAsset.generated.h"

/**
 * Contains configuration data for walls.
 * Content is stored in FBmrWallRow Data Registry rows
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