// Copyright (c) Yevhenii Selivanov.

#include "UI/InGameWidget.h"
//---
#include "SoundsManager.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "UI/MyHUD.h"
#include "UI/SettingsWidget.h"
//---
#include "Components/Button.h"

// Shows the in game menu.
void UInGameWidget::ShowInGameMenu_Implementation()
{
	if (IsVisibleInGameMenu())
	{
		// Is already shown
		return;
	}

	OnToggleInGameMenu(true);

	// Blueprint implementation
	// ...
}

// Hide the in game menu.
void UInGameWidget::HideInGameMenu_Implementation()
{
	if (!IsVisibleInGameMenu())
	{
		// Is already hidden
		return;
	}

	OnToggleInGameMenu(false);

	// Blueprint implementation
	// ...
}

// Flip-floppy show and hide the end game state window
void UInGameWidget::ToggleInGameMenu()
{
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState(GetWorld());
	if (CurrentGameState != ECurrentGameState::InGame
	    && CurrentGameState != ECurrentGameState::EndGame)
	{
		return;
	}

	if (IsVisibleInGameMenu())
	{
		HideInGameMenu();
	}
	else
	{
		ShowInGameMenu();
	}
}

// Is called when In-Game menu became opened or closed
void UInGameWidget::OnToggleInGameMenu(bool bIsVisible)
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	if (OnToggledInGameMenu.IsBound())
	{
		OnToggledInGameMenu.Broadcast(bIsVisible);
	}
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UInGameWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen changing the visibility of this widget
	OnVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnWidgetVisibilityChanged);

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		BindOnGameStateChanged(MyGameState);
	}
	else if (AMyPlayerController* MyPC = USingletonLibrary::GetLocalPlayerController())
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::BindOnGameStateChanged);
	}

	// Listen to toggle the game state widget when is requested
	if (AMyHUD* MyHUD = USingletonLibrary::GetMyHUD())
	{
		MyHUD->OnClose.AddUniqueDynamic(this, &ThisClass::ToggleInGameMenu);
	}

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

// Updates appearance dynamically in the editor
void UInGameWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

// Launch 'Three-two-one-GO' timer.
void UInGameWidget::LaunchStartingCountdown_Implementation()
{
	// Blueprint implementation
	// ...
}

// Launch the main timer that count the seconds to the game ending.
void UInGameWidget::LaunchInGameCountdown_Implementation()
{
	// Blueprint implementation
	// ...
}

// Called when the visibility of this widgets was changed
void UInGameWidget::OnWidgetVisibilityChanged(ESlateVisibility InVisibility)
{
	if (InVisibility != ESlateVisibility::Visible)
	{
		HideInGameMenu();
	}
}

// Called when the current game state was changed
void UInGameWidget::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		{
			SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
		case ECurrentGameState::GameStarting:
		{
			SetVisibility(ESlateVisibility::Visible);
			HideInGameMenu();
			LaunchStartingCountdown();
			break;
		}
		case ECurrentGameState::EndGame:
		{
			ShowInGameMenu();
			break;
		}
		case ECurrentGameState::InGame:
		{
			LaunchInGameCountdown();
			HideInGameMenu();
			break;
		}
		default:
			break;
	}
}

// Is called when player pressed the button to restart the game
void UInGameWidget::OnRestartButtonPressed()
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	if (AMyPlayerController* MyPC = USingletonLibrary::GetLocalPlayerController())
	{
		MyPC->SetGameStartingState();
	}
}

// Is called when player pressed the button to go back to the Main Menu
void UInGameWidget::OnMenuButtonPressed()
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	if (AMyPlayerController* MyPC = USingletonLibrary::GetLocalPlayerController())
	{
		MyPC->SetMenuState();
	}
}

// Is called when player pressed the button to open in-game Settings
void UInGameWidget::OnSettingsButtonPressed()
{
	if (USettingsWidget* SettingsWidget = USingletonLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}

// Is called to start listening game state changes
void UInGameWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	if (!ensureMsgf(MyGameState, TEXT("ASSERT: 'MyGameState' is not valid")))
	{
		return;
	}

	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}
