// Copyright (c) Yevhenii Selivanov

#include "UtilityLibraries/MyViewModelUtilsLibrary.h"
//---
#include "Bomber.h"
//---
#include "Components/SlateWrapperTypes.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyViewModelUtilsLibrary)

// Is used a lot by the UI View Models as 'Conversion Function' to show or hide own widget
ESlateVisibility UMyViewModelUtilsLibrary::GetVisibilityByGameState(const ECurrentGameState& GameStateProperty, int32 GameStates)
{
	const bool bMatching = EnumHasAnyFlags(GameStateProperty, TO_ENUM(ECurrentGameState, GameStates));
	return bMatching ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}