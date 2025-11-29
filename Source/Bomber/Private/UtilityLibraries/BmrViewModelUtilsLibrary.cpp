// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/MyViewModelUtilsLibrary.h"

// Bomber
#include "Bomber.h"

// UE
#include "Components/SlateWrapperTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MyViewModelUtilsLibrary)

// Is used a lot by the UI View Models as 'Conversion Function' to show or hide own widget
ESlateVisibility UMyViewModelUtilsLibrary::GetVisibilityByGameState(const ECurrentGameState& GameStateProperty, int32 GameStates)
{
	const bool bMatching = EnumHasAnyFlags(GameStateProperty, TO_ENUM(ECurrentGameState, GameStates));
	return bMatching ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
}

// Used widely by UI View Models as a 'Conversion Function' to determine state-based activity
bool UMyViewModelUtilsLibrary::IsGameStateMatching(const ECurrentGameState& GameStateProperty, int32 GameStates)
{
	return EnumHasAnyFlags(GameStateProperty, TO_ENUM(ECurrentGameState, GameStates));
}