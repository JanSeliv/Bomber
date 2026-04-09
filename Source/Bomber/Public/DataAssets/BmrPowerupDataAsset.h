// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrPowerupDataAsset.generated.h"

/**
 * Contains configuration data for powerups.
 * Content is stored in FBmrPowerupRow Data Registry rows
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPowerupDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrPowerupDataAsset();

	/** Returns the powerup data asset. */
	static const UBmrPowerupDataAsset& Get();
};
