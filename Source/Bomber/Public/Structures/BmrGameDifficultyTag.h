// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameplayTagContainer.h"

// UE
#include "Engine/DataTable.h" // FTableRowBase

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

/**
 * Row struct for difficulty data table registered via Data Registry.
 * Mods register their own data tables with custom difficulty rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrGameDifficultyRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The gameplay tag representing this difficulty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrGameDifficultyTag DifficultyTag = FBmrGameDifficultyTag::None;

	/** Integer for settings combobox index and curve table evaluation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DifficultyLevel = INDEX_NONE;

	/** Localized display name shown in settings combobox. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName = FText::GetEmpty();
};