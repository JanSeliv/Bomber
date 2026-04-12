// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"

#include "BmrMapTag.generated.h"

/**
 * The tag that represents a map visual theme.
 * Is designed to be extensible by Modular Game Features (GameFeatures/Maps/...)
 */
USTRUCT(BlueprintType, meta = (Categories = "Map"))
struct BOMBER_API FBmrMapTag : public FGameplayTag
{
	GENERATED_BODY()

	/** Default constructor. */
	FBmrMapTag() = default;

	/** Custom constructor to set all members values. */
	FBmrMapTag(const FGameplayTag& Tag);

	/** The Map tag that contains nothing chosen by default. */
	static const FBmrMapTag None;

	/** Parent tag for all map visual theme tags. */
	static const FGameplayTag ParentTag;
};
