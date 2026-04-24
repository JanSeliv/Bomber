// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"

#include "BmrPlayerTag.generated.h"

/**
 * The tag that represents all available player characters in game.
 */
USTRUCT(BlueprintType, meta = (Categories = "Player"))
struct BOMBER_API FBmrPlayerTag : public FGameplayTag
{
	GENERATED_BODY()

	/** Default constructor. */
	FBmrPlayerTag() = default;

	/** Custom constructor to set all members values. */
	FBmrPlayerTag(const FGameplayTag& Tag);

	/** The Player Character tag that contains nothing chosen by default. */
	static const FBmrPlayerTag None;

	/** Fallback character tag, each map declares its own character under this tag, is often used by AI characters, or if all characters are shared. */
	static const FBmrPlayerTag Default;
};