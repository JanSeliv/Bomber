// Copyright (c) Yevhenii Selivanov.

#include "UI/InGameWidget.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InGameWidget)

// Called after the underlying slate widget is constructed. May be called multiple times due to adding and removing from the hierarchy.
void UInGameWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		BindOnGameStateChanged(MyGameState);
	}
	else if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::BindOnGameStateChanged);
	}
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
			LaunchStartingCountdown();
			break;
		}
		case ECurrentGameState::EndGame:
		{
			break;
		}
		case ECurrentGameState::InGame:
		{
			LaunchInGameCountdown();
			break;
		}
		default:
			break;
	}
}

// Is called to start listening game state changes
void UInGameWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	checkf(MyGameState, TEXT("ERROR: 'MyGameState' is null!"));

	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}
}
