// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/BmrTagUtilsLibrary.h"

// Bomber
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrPlayerTag.h"
#include "Structures/BmrPowerupTag.h"

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

// Converts a GameStateTag to a GameplayTag
FBmrGameStateTag UBmrTagUtilsLibrary::Conv_GameplayTagToGameStateTag(FGameplayTag InGameplayTag)
{
	return FBmrGameStateTag(InGameplayTag);
}

// Converts a GameplayTag to a GameStateTag
FGameplayTag UBmrTagUtilsLibrary::Conv_GameStateTagToGameplayTag(FBmrGameStateTag InGameStateTag)
{
	return InGameStateTag;
}
