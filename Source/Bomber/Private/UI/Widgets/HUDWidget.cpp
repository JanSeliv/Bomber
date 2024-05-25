// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/HUDWidget.h"
//---
#include "GameFramework/MyPlayerState.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Animation/WidgetAnimation.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(HUDWidget)

//  Called after the underlying slate widget is constructed
void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the local player state is initialized and its assigned character is ready
void UHUDWidget::OnLocalPlayerStateReady_Implementation(class AMyPlayerState* PlayerState, int32 CharacterID)
{
	// Listen the ending the current game to play the End-Game sound on
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Is called on end-game result change
void UHUDWidget::OnEndGameStateChanged_Implementation(EEndGameState EndGameState)
{
	if (EndGameState != EEndGameState::None)
	{
		PlayAnimation(ResultAnimation);
	}
}