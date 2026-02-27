// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"

#include "DalPrimaryDataAsset.generated.h"

/**
 * Is base class for data assets that self-register in UDalSubsystem via PostLoad
 * These data assets are loaded automatically by Asset Registry if setup in one of the following ways:
 * - Project data assets are discovered by Project Settings -> Asset Manager -> PrimaryAssetTypesToScan.
 * - Modular Game Feature data assets are discovered via GameFeatureData asset -> PrimaryAssetTypesToScan.
 */
UCLASS(Abstract)
class DATAASSETSLOADER_API UDalPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** All subclasses share the same primary asset type so Asset Manager discovers them under a single type */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Self-registers in UDalSubsystem regardless of how it was loaded: Asset Manager async loading, strong (hard) reference, synchronously etc */
	virtual void PostLoad() override;

	/** Unregisters this data asset from UDalSubsystem before destruction */
	virtual void BeginDestroy() override;
};
