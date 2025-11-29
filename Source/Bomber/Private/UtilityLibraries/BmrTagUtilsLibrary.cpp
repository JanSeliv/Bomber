// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/MyTagUtilsLibrary.h"

// Bomber
#include "Structures/BmrPowerupTag.h"
#include "Structures/PlayerTag.h"

// UE
#include "GameplayTagContainer.h"

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

// Converts a PowerupTag to a GameplayTag
FBmrPowerupTag UMyTagUtilsLibrary::Conv_GameplayTagToPowerupTag(FGameplayTag InGameplayTag)
{
	return FBmrPowerupTag(InGameplayTag);
}

// Converts a GameplayTag to a PowerupTag
FGameplayTag UMyTagUtilsLibrary::Conv_PowerupTagToGameplayTag(FBmrPowerupTag InPowerupTag)
{
	return InPowerupTag;
}
