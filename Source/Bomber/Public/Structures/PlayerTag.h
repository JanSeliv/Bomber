// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"
//---
#include "PlayerTag.generated.h"

/**
 * The tag that represents all available player characters in game.
 */
USTRUCT(BlueprintType, meta = (Categories = "Player"))
struct BOMBER_API FPlayerTag : public FGameplayTag
{
	GENERATED_BODY()

	/** Default constructor. */
	FPlayerTag() = default;

	/** Custom constructor to set all members values. */
	FPlayerTag(const FGameplayTag& Tag);

	/** The Player Character tag that contains nothing chosen by default. */
	static const FPlayerTag None;
};