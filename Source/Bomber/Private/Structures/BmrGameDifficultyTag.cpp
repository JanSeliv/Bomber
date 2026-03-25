// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrGameDifficultyTag.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGameDifficultyTag)

// Empty tag, nothing chosen by default
const FBmrGameDifficultyTag FBmrGameDifficultyTag::None = EmptyTag;

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Difficulty, "Difficulty", "Parent tag for all difficulty tags");
const FGameplayTag FBmrGameDifficultyTag::ParentTag = TAG_Difficulty.GetTag();

// Custom constructor to set all members values
FBmrGameDifficultyTag::FBmrGameDifficultyTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}