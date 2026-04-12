// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrMapTag.h"

// UE
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMapTag)

// The Map tag that contains nothing chosen by default
const FBmrMapTag FBmrMapTag::None = EmptyTag;

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Map, "Map", "Parent tag for all map visual themes");
const FGameplayTag FBmrMapTag::ParentTag = TAG_Map.GetTag();

// Custom constructor to set all members values
FBmrMapTag::FBmrMapTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}
