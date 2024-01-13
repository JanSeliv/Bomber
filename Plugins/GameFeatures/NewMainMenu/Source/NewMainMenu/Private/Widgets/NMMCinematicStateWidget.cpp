// Copyright (c) Yevhenii Selivanov

#include "Widgets/NMMCinematicStateWidget.h"
//---
#include "Bomber.h"
#include "Components/MouseActivityComponent.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Button.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCinematicStateWidget)

// // Called after the underlying slate widget is constructed
void UNMMCinematicStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide this widget by default
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

	if (SkipCinematicButton)
	{
		SkipCinematicButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		SkipCinematicButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSkipCinematicButtonPressed);
	}

	if (UMouseActivityComponent* MouseActivityComponent = UMyBlueprintFunctionLibrary::GetMouseActivityComponent())
	{
		MouseActivityComponent->OnMouseVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnMouseVisibilityChanged);
	}
}

// Called when the current game state was changed
void UNMMCinematicStateWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show this widget in Cinematic state
	SetVisibility(CurrentGameState == ECurrentGameState::Cinematic ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called to start listening game state changes
void UNMMCinematicStateWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	checkf(MyGameState, TEXT("ERROR: 'MyGameState' is null!"));

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}
}

// Is called to skip cinematic
void UNMMCinematicStateWidget::OnSkipCinematicButtonPressed()
{
	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->SetGameStartingState();
	}
}

// Is bound to toggle 'SkipCinematicButton' visibility when mouse became shown or hidden
void UNMMCinematicStateWidget::OnMouseVisibilityChanged_Implementation(bool bIsShown)
{
	checkf(SkipCinematicButton, TEXT("ERROR: [%i] %s:\n'SkipCinematicButton' is null!"), __LINE__, *FString(__FUNCTION__));
	SkipCinematicButton->SetVisibility(bIsShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
