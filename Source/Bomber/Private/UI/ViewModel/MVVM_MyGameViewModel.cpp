// Copyright (c) Yevhenii Selivanov

#include "UI/ViewModel/MVVM_MyGameViewModel.h"
//---
#include "Subsystems/GlobalEventsSubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_MyGameViewModel)

// Setter and Getter widgets about the current game state
void UMVVM_MyGameViewModel::SetCurrentGameState(ECurrentGameState NewCurrentGameState)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentGameState, NewCurrentGameState);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called when the view is constructed
void UMVVM_MyGameViewModel::OnViewModelConstruct_Implementation(const UUserWidget* UserWidget)
{
	Super::OnViewModelConstruct_Implementation(UserWidget);

	UGlobalEventsSubsystem::Get().OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::SetCurrentGameState);
}

// Is called when this View Model is destructed
void UMVVM_MyGameViewModel::OnViewModelDestruct_Implementation()
{
	Super::OnViewModelDestruct_Implementation();

	if (UGlobalEventsSubsystem* GlobalEventsSubsystem = UGlobalEventsSubsystem::GetGlobalEventsSubsystem())
	{
		GlobalEventsSubsystem->OnGameStateChanged.RemoveAll(this);
	}
}