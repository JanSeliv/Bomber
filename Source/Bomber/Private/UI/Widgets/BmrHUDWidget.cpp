// Copyright (c) Yevhenii Selivanov

#include "UI/Widgets/BmrHUDWidget.h"

// Bomber
#include "Controllers/BmrPlayerController.h"
#include "GameFramework/BmrPlayerState.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UI/SettingsWidget.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrHUDWidget)

//  Called after the underlying slate widget is constructed
void UBmrHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);

	if (RestartButton)
	{
		RestartButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		RestartButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnRestartButtonPressed);
	}

	if (MenuButton)
	{
		MenuButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		MenuButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnMenuButtonPressed);
	}

	if (SettingsButton)
	{
		SettingsButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		SettingsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSettingsButtonPressed);
	}
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

// Is called when player pressed the button to restart the match
void UBmrHUDWidget::OnRestartButtonPressed_Implementation()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!MyPC)
	{
		return;
	}

	MyPC->SetGameStartingState();
}

// Is called when player pressed the button to return to the Main Menu
void UBmrHUDWidget::OnMenuButtonPressed_Implementation()
{
	ABmrPlayerController* MyPC = GetOwningPlayer<ABmrPlayerController>();
	if (!MyPC)
	{
		return;
	}

	MyPC->SetMenuState();
}

// Is called when player pressed the button to open the Settings
void UBmrHUDWidget::OnSettingsButtonPressed_Implementation()
{
	if (USettingsWidget* SettingsWidget = UBmrBlueprintFunctionLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}