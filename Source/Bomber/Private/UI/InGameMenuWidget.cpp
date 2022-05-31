﻿// Copyright (c) Yevhenii Selivanov.

#include "UI/InGameMenuWidget.h"
//---
#include "SoundsManager.h"
#include "Controllers/MyPlayerController.h"
#include "Globals/SingletonLibrary.h"
#include "UI/SettingsWidget.h"
#include "UI/MyHUD.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
//---
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Returns true if the In-Game widget is shown on user screen
bool UInGameMenuWidget::IsVisibleInGameMenu() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UInGameMenuWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>();
	if (!ensureMsgf(MyPC, TEXT("ASSERT: 'MyPC' is not valid")))
	{
		return;
	}

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen changing the game states to handle In-Game Menu visibility
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		BindOnGameStateChanged(MyGameState);
	}
	else
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::BindOnGameStateChanged);
	}

	// Listen to update the End-Game state tex;
	if (AMyPlayerState* MyPlayerState = MyPC->GetPlayerState<AMyPlayerState>())
	{
		BindOnEndGameStateChanged(MyPlayerState);
	}
	else
	{
		MyPC->OnSetPlayerState.AddUniqueDynamic(this, &ThisClass::BindOnEndGameStateChanged);
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

// Called when the current game state was changed
void UInGameMenuWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:   // fallthrough
		case ECurrentGameState::InGame: // fallthrough
		case ECurrentGameState::GameStarting:
		{
			HideInGameMenu();
			break;
		}
		case ECurrentGameState::EndGame:
		{
			ShowInGameMenu();
			break;
		}
		default:
			break;
	}
}

// Called when the end-game state was changed
void UInGameMenuWidget::OnEndGameStateChanged(EEndGameState EndGameState)
{
	if (EndGameState != EEndGameState::None)
	{
		ShowInGameMenu();
	}
}

// Is called to start listening game state changes
void UInGameMenuWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	if (!ensureMsgf(MyGameState, TEXT("ASSERT: 'MyGameState' is not valid")))
	{
		return;
	}

	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}

// Is called to start listening End-Game state changes
void UInGameMenuWidget::BindOnEndGameStateChanged(AMyPlayerState* MyPlayerState)
{
	if (!ensureMsgf(MyPlayerState, TEXT("ASSERT: 'MyPlayerState' is not valid")))
	{
		return;
	}

	MyPlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Is called when player pressed the button to restart the game
void UInGameMenuWidget::OnRestartButtonPressed()
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->SetGameStartingState();
	}
}

// Is called when player pressed the button to go back to the Main Menu
void UInGameMenuWidget::OnMenuButtonPressed()
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->SetMenuState();
	}
}

// Is called when player pressed the button to open in-game Settings
void UInGameMenuWidget::OnSettingsButtonPressed()
{
	if (USettingsWidget* SettingsWidget = USingletonLibrary::GetSettingsWidget())
	{
		SettingsWidget->OpenSettings();
	}
}

// Called to set Win,Draw or lose text on UI
void UInGameMenuWidget::UpdateEndGameText()
{
	const APlayerController* PC = GetOwningPlayer();
	const AMyPlayerState* MyPlayerState = PC ? PC->GetPlayerState<AMyPlayerState>() : nullptr;
	if (!ensureMsgf(MyPlayerState, TEXT("ASSERT: 'MyPlayerState' is not valid"))
	    || !ensureMsgf(EndGameTextWidget, TEXT("ASSERT: 'MyPlayerState' is not valid")))
	{
		return;
	}

	FText EndGameText = FText::GetEmpty();
	const EEndGameState EndGameState = MyPlayerState->GetEndGameState();
	if (EndGameState != EEndGameState::None)
	{
		EndGameText = UUIDataAsset::Get().GetEndGameText(EndGameState);
	}

	EndGameTextWidget->SetText(EndGameText);
}

// Shows the in game menu.
void UInGameMenuWidget::ShowInGameMenu()
{
	if (IsVisibleInGameMenu())
	{
		// Is already shown
		return;
	}

	UpdateEndGameText();

	OnToggleInGameMenu(true);

	// Blueprint implementation
	// ...
}

// Hide the in game menu.
void UInGameMenuWidget::HideInGameMenu()
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
void UInGameMenuWidget::ToggleInGameMenu()
{
	const ECurrentGameState CurrentGameState = AMyGameStateBase::GetCurrentGameState(this);
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
void UInGameMenuWidget::OnToggleInGameMenu(bool bIsVisible)
{
	const ESlateVisibility NewVisibility = bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	SetVisibility(NewVisibility);

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
