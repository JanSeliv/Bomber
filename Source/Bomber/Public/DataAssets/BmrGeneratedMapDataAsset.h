// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"

// Bomber
#include "Structures/GeneratedMapSettings.h"

#include "GeneratedMapDataAsset.generated.h"

/**
 * Contains all data that describe all levels.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UGeneratedMapDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the generated map data asset. */
	static const UGeneratedMapDataAsset& Get();

	/** Returns settings for runtime generation of the level map.
	 * Prefer AGeneratedMap::GetGenerationSetting() instead, as defaults might be overridden by Class Defaults of the Generated Map itself. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FGeneratedMapSettings& GetGenerationSettings() const { return GenerationSettingsInternal; }

	/** Get UGeneratedMapDataAsset::CollisionsAssetInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class AActor> GetCollisionsAssetClass() const { return CollisionsAssetInternal; }

protected:
	/** Contains settings for runtime generation of the level map.
	 * These settings might be overridden by Class Defaults of the Generated Map itself. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Generation Settings", ShowOnlyInnerProperties))
	FGeneratedMapSettings GenerationSettingsInternal;

	/** Asset that contains scalable collision. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collisions Asset", ShowOnlyInnerProperties))
	TSubclassOf<class AActor> CollisionsAssetInternal = nullptr;
};