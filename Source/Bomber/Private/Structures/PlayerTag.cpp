// Copyright (c) Yevhenii Selivanov

#include "Structures/PlayerTag.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerTag)

// The Player Character tag that contains nothing chosen by default
const FPlayerTag FPlayerTag::None = EmptyTag;

// Custom constructor to set all members values
FPlayerTag::FPlayerTag(const FGameplayTag& Tag)
	: FGameplayTag(Tag) {}
