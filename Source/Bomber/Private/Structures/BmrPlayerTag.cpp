// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrPlayerTag.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerTag)

// The Player Character tag that contains nothing chosen by default
const FBmrPlayerTag FBmrPlayerTag::None = EmptyTag;

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Default, "Player.Default", "Fallback character used by the active map as its default human or AI character");
const FBmrPlayerTag FBmrPlayerTag::Default = TAG_Player_Default.GetTag();

// Custom constructor to set all members values
FBmrPlayerTag::FBmrPlayerTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}
