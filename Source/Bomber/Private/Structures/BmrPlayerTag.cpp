// Copyright (c) Yevhenii Selivanov

#include "Structures/BmrPlayerTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPlayerTag)

// The Player Character tag that contains nothing chosen by default
const FBmrPlayerTag FBmrPlayerTag::None = EmptyTag;

// Custom constructor to set all members values
FBmrPlayerTag::FBmrPlayerTag(const FGameplayTag& Tag)
    : FGameplayTag(Tag)
{
}
