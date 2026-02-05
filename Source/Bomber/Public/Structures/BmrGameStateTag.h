// Copyright (c) Yevhenii Selivanov

#pragma once

// UE
#include "GameplayTagContainer.h"

#include "BmrGameStateTag.generated.h"

/**
 * The tag that represents specific game state type.
 */
USTRUCT(BlueprintType, meta = (Categories = "GameState"))
struct BOMBER_API FBmrGameStateTag : public FGameplayTag
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * State Tags
	 ********************************************************************************************* */

	static const FBmrGameStateTag None; // Empty tag, nothing chosen by default
	static const FGameplayTag ParentTag; // Parent tag for binding to tag changes and filtering
	static const FBmrGameStateTag Menu; // Is active while players are in Main-Menu
	static const FBmrGameStateTag GameStarting; // Is active while players see count-down (3-2-1)
	static const FBmrGameStateTag InGame; // Is active during the match
	static const FBmrGameStateTag EndGame; // Is active when match is finished

	/*********************************************************************************************
	 * Logic
	 ********************************************************************************************* */

	/** Default constructor. */
	FBmrGameStateTag() = default;

	/** Custom constructor to set all members values. */
	FBmrGameStateTag(const FGameplayTag& Tag);

	/** Combines two tags into a container for use with HasAny(), e.g.: TagContainer.HasAny(Tag1 | Tag2). */
	friend BOMBER_API FGameplayTagContainer operator|(const FBmrGameStateTag& A, const FBmrGameStateTag& B);

	/** Allows chaining multiple tags, e.g.: TagContainer.HasAny(Tag1 | Tag2 | Tag3). */
	friend BOMBER_API FGameplayTagContainer operator|(const FGameplayTagContainer& Container, const FBmrGameStateTag& Tag);
};
