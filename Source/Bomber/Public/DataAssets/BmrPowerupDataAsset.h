// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "BmrPowerupDataAsset.generated.h"

/**
 * Row that describes each unique item.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPowerupRow final : public UBmrLevelActorRow
{
	GENERATED_BODY()

public:
	/** Of each type this item is. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	FBmrPowerupTag ItemType = FBmrPowerupTag::None;

	/** Gameplay effect to apply on collecting this powerup to change the attributes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	TSubclassOf<class UGameplayEffect> CollectGameplayEffect = nullptr;
};

/**
 * Describes common data for all items.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrPowerupDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrPowerupDataAsset();

	/** Returns the item data asset. */
	static const UBmrPowerupDataAsset& Get();

	/** Return row by specified item type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const UBmrPowerupRow* GetRowByItemType(FBmrPowerupTag ItemType, EBmrLevelType LevelType) const;
};