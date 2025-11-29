// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "ItemDataAsset.generated.h"

/**
 * Row that describes each unique item.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UItemRow final : public ULevelActorRow
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
class BOMBER_API UItemDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UItemDataAsset();

	/** Returns the item data asset. */
	static const UItemDataAsset& Get();

	/** Return row by specified item type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const UItemRow* GetRowByItemType(FBmrPowerupTag ItemType, ELevelType LevelType) const;
};