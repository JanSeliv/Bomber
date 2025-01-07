// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"
//---
#include "Structures/GeneratedMapSettings.h"
//---
#include "GeneratedMapDataAsset.generated.h"

/**
 * Unique data about one separated level.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FLevelStreamRow
{
	GENERATED_BODY()

	/** The empty mesh row. */
	static const FLevelStreamRow Empty;

	/** Default constructor */
	FLevelStreamRow() = default;

	/** The level asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<class UWorld> Level = nullptr;

	/** The associated type of a level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	ELevelType LevelType = ELT::None;

	/** The name of a level on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	FText LevelName = TEXT_NONE;
};

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

	/** Returns level rows. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetLevelStreamRows(TArray<FLevelStreamRow>& OutRows) const { OutRows = LevelsInternal; }

	/** Get UGeneratedMapDataAsset::CollisionsAssetInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class AActor> GetCollisionsAssetClass() const { return CollisionsAssetInternal; }

protected:
	/** Contains settings for runtime generation of the level map.
	 * These settings might be overridden by Class Defaults of the Generated Map itself. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Generation Settings", ShowOnlyInnerProperties))
	FGeneratedMapSettings GenerationSettingsInternal;

	/** Contains all used levels. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels", TitleProperty = "LevelType", ShowOnlyInnerProperties))
	TArray<FLevelStreamRow> LevelsInternal;

	/** Asset that contains scalable collision. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collisions Asset", ShowOnlyInnerProperties))
	TSubclassOf<class AActor> CollisionsAssetInternal = nullptr;
};