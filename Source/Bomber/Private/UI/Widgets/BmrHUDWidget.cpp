// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/BmrHUDWidget.h"

// Bomber
#include "GameFramework/BmrPlayerState.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Animation/WidgetAnimation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrHUDWidget)

//  Called after the underlying slate widget is constructed
void UBmrHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the local player state is initialized and its assigned character is ready
void UBmrHUDWidget::OnLocalPlayerStateReady_Implementation(class ABmrPlayerState* PlayerState, int32 PlayerId)
{
	// Listen the ending the current game to play the End-Game sound on
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Is called on end-game result change
void UBmrHUDWidget::OnEndGameStateChanged_Implementation(EBmrEndGameState EndGameState)
{
	if (EndGameState != EBmrEndGameState::None)
	{
		PlayAnimation(ResultAnimation);
	}
}