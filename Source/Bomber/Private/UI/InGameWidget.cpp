// Copyright 2021 Yevhenii Selivanov.

#include "UI/InGameWidget.h"
//---
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"

// Shows the in game menu.
void UInGameWidget::ShowEndGameState_Implementation()
{
	// Show mouse cursor
	if (AMyPlayerController* MyPlayerController = USingletonLibrary::GetMyPlayerController())
	{
		MyPlayerController->SetMouseCursor(true);
	}

	// Blueprint implementation
	// ...
}

void UInGameWidget::HideEndGameState_Implementation()
{
	// Hide mouse cursor
	if (AMyPlayerController* MyPlayerController = USingletonLibrary::GetMyPlayerController())
	{
		MyPlayerController->SetMouseCursor(false);
	}

	// Blueprint implementation
	// ...
}

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UInGameWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
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

//
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
			HideEndGameState();
			LaunchStartingCountdown();
			break;
		}
		case ECurrentGameState::EndGame:
		{
			ShowEndGameState();
			break;
		}
		case ECurrentGameState::InGame:
		{
			LaunchInGameCountdown();
			HideEndGameState();
			break;
		}
		default: break;
	}
}
