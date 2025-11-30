// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/BmrTagUtilsLibrary.h"

// Bomber
#include "Structures/BmrPowerupTag.h"
#include "Structures/BmrPlayerTag.h"

// UE
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrTagUtilsLibrary)

// Converts a PlayerTag to a GameplayTag
FBmrPlayerTag UBmrTagUtilsLibrary::Conv_GameplayTagToPlayerTag(FGameplayTag InGameplayTag)
{
	return FBmrPlayerTag(InGameplayTag);
}

// Converts a GameplayTag to a PlayerTag
FGameplayTag UBmrTagUtilsLibrary::Conv_PlayerTagToGameplayTag(FBmrPlayerTag InPlayerTag)
{
	return InPlayerTag;
}

// Converts a PowerupTag to a GameplayTag
FBmrPowerupTag UBmrTagUtilsLibrary::Conv_GameplayTagToPowerupTag(FGameplayTag InGameplayTag)
{
	return FBmrPowerupTag(InGameplayTag);
}

// Converts a GameplayTag to a PowerupTag
FGameplayTag UBmrTagUtilsLibrary::Conv_PowerupTagToGameplayTag(FBmrPowerupTag InPowerupTag)
{
	return InPowerupTag;
}
