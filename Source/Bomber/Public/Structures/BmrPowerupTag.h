// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameplayTagContainer.h"

#include "BmrPowerupTag.generated.h"

/**
 * The tag that represents specific powerup type.
 */
USTRUCT(BlueprintType, DisplayName = "Powerup Tag", meta = (Categories = "Powerup"))
struct BOMBER_API FBmrPowerupTag : public FGameplayTag
{
	GENERATED_BODY()

	static const FBmrPowerupTag None; // Empty tag, nothing chosen by default
	static const FBmrPowerupTag Skate; // Increases the player's movement speed
	static const FBmrPowerupTag Bomb; // Increases the number of bombs that can be placed at once
	static const FBmrPowerupTag Fire; // Increases the explosion radius of bombs

	/** Returns all powerup tags gathered from Data Registry, falls back to hardcoded tags if DR is not loaded yet */
	static FGameplayTagContainer GetAll();

	/** Default constructor. */
	FBmrPowerupTag() = default;

	/** Custom constructor to set all members values. */
	FBmrPowerupTag(const FGameplayTag& Tag);
};