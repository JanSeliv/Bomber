// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"
//---
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
	EItemType ItemType = EItemType::None;
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

	/** Returns speed value that is added to the player speed on taking a skate item.
	  * @see UItemDataAsset::SkateStrengthInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetSkateAdditiveStrength() const { return SkateAdditiveStrengthInternal; }

	/** Return row by specified item type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UItemRow* GetRowByItemType(EItemType ItemType, ELevelType LevelType) const;

	/** Returns max possible items to be picked up by player.
	  * @see UItemDataAsset::MaxAllowedItemNumInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMaxAllowedItemsNum() const { return MaxAllowedItemsNumInternal; }

protected:
	/** The speed additive value when player takes the skate item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skate Additive Strength", ShowOnlyInnerProperties))
	float SkateAdditiveStrengthInternal = 500.f;

	/** Max possible items to be picked up by player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Max Allowed Items Num", ShowOnlyInnerProperties, ClampMin = "1", UIMin = "1"))
	int32 MaxAllowedItemsNumInternal = 5;
};