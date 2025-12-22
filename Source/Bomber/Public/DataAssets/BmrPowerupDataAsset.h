// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "BmrPowerupDataAsset.generated.h"

/**
 * Row that describes each unique powerup.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPowerupRow final : public UBmrLevelActorRow
{
	GENERATED_BODY()

public:
	/** Of each type this powerup is. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	FBmrPowerupTag PowerupTag = FBmrPowerupTag::None;

	/** Gameplay effect to apply on collecting this powerup to change the attributes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	TSubclassOf<class UGameplayEffect> CollectGameplayEffect = nullptr;

	/** Gameplay effect to apply when the max amount of this powerup is reached. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	TSubclassOf<class UGameplayEffect> MaxCollectGameplayEffect = nullptr;
};

/**
 * Describes common data for all powerups.
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

	/** Return row by specified powerup type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrPowerupRow* GetRowByPowerupTag(FBmrPowerupTag Tag, EBmrLevelType LevelType = EBmrLevelType::None) const;
};