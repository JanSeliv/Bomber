// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"

#include "BmrGameDifficultyTag.generated.h"

/**
 * The tag that represents a game difficulty, extensible via Data Registry.
 */
USTRUCT(BlueprintType, DisplayName = "Difficulty Tag", meta = (Categories = "Difficulty"))
struct BOMBER_API FBmrGameDifficultyTag : public FGameplayTag
{
	GENERATED_BODY()

	/** Default constructor. */
	FBmrGameDifficultyTag() = default;

	/** Custom constructor to set all members values. */
	FBmrGameDifficultyTag(const FGameplayTag& Tag);

	/** Empty tag, nothing chosen by default. */
	static const FBmrGameDifficultyTag None;

	/** Parent tag for all difficulty tags. */
	static const FGameplayTag ParentTag;
};
