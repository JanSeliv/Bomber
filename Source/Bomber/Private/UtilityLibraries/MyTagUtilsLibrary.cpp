// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/MyTagUtilsLibrary.h"
//---
#include "Structures/PlayerTag.h"
//---
#include "GameplayTagContainer.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyTagUtilsLibrary)

// Converts a PlayerTag to a GameplayTag
FPlayerTag UMyTagUtilsLibrary::Conv_GameplayTagToPlayerTag(FGameplayTag InGameplayTag)
{
	return FPlayerTag(InGameplayTag);
}

// Converts a GameplayTag to a PlayerTag
FGameplayTag UMyTagUtilsLibrary::Conv_PlayerTagToGameplayTag(FPlayerTag InPlayerTag)
{
	return InPlayerTag;
}