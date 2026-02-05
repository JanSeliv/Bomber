// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/BmrViewModelUtilsLibrary.h"

// Bomber
#include "Structures/BmrGameStateTag.h"

// UE
#include "Components/SlateWrapperTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrViewModelUtilsLibrary)

// Is used to show or hide own widget by the current game state tag
ESlateVisibility UBmrViewModelUtilsLibrary::GetVisibilityByGameStateTag(const FBmrGameStateTag& CurrentGameStateTag, const FGameplayTagContainer& GameStates)
{
	const bool bMatching = CurrentGameStateTag.GetSingleTagContainer().HasAny(GameStates);
	return bMatching ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
}

// Checks if the current game state tag matches any of the specified tags
bool UBmrViewModelUtilsLibrary::IsGameStateTagMatching(const FBmrGameStateTag& CurrentGameStateTag, const FGameplayTagContainer& GameStates)
{
	return CurrentGameStateTag.GetSingleTagContainer().HasAny(GameStates);
}