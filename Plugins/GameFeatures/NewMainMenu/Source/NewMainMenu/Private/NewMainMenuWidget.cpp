// Copyright (c) Yevhenii Selivanov

#include "NewMainMenuWidget.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Button.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NewMainMenuWidget)

// Called after the underlying slate widget is constructed
void UNewMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState();
	checkf(MyGameState, TEXT("%s: 'MyGameState' is null"), *FString(__FUNCTION__));

	// Handle current game state if initialized with delay
	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		OnGameStateChanged(ECurrentGameState::Menu);
	}

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

	if (PlayButton)
	{
		PlayButton->SetClickMethod(EButtonClickMethod::PreciseClick);
		PlayButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPlayButtonPressed);
	}
}

// Called when the current game state was changed
void UNewMainMenuWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show this widget in Menu game state
	SetVisibility(CurrentGameState == ECurrentGameState::Menu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called when player pressed the button to start the game
void UNewMainMenuWidget::OnPlayButtonPressed()
{
	USoundsSubsystem::Get().PlayUIClickSFX();

	if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->ServerSetGameState(ECurrentGameState::Cinematic);
	}
}
