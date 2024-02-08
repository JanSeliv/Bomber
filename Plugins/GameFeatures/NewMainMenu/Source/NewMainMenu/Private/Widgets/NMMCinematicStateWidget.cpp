// Copyright (c) Yevhenii Selivanov

#include "Widgets/NMMCinematicStateWidget.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/Button.h"
#include "Components/RadialSlider.h"
#include "Components/TextBlock.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCinematicStateWidget)

// Applies the given time to hold the skip progress to skip the cinematic
void UNMMCinematicStateWidget::SetCurrentHoldTime(float NewHoldTime)
{
	CurrentHoldTimeInternal = NewHoldTime;

	const float MaxHoldTime = UNMMDataAsset::Get().GetSkipCinematicHoldTime();
	const float HoldProgressNormalized = FMath::Clamp(CurrentHoldTimeInternal / MaxHoldTime, 0.f, 1.f);

	checkf(SkipHoldProgress, TEXT("ERROR: [%i] %s:\n'SkipHoldProgress' is null!"), __LINE__, *FString(__FUNCTION__));
	SkipHoldProgress->SetValue(HoldProgressNormalized);

	const UWorld* World = GetWorld();
	checkf(World, TEXT("ERROR: [%i] %s:\n'World' is null!"), __LINE__, *FString(__FUNCTION__));
	if (CurrentHoldTimeInternal > 0.f && CurrentHoldTimeInternal <= World->GetDeltaSeconds())
	{
		OnCinematicSkipStarted();
	}
	else if (CurrentHoldTimeInternal >= MaxHoldTime)
	{
		OnCinematicSkipFinished();
	}
}

// Reset to default state
void UNMMCinematicStateWidget::ResetWidget()
{
	SetCurrentHoldTime(0.f);

	checkf(SkipText, TEXT("ERROR: [%i] %s:\n'SkipText' is null!"), __LINE__, *FString(__FUNCTION__));
	SkipText->SetVisibility(ESlateVisibility::Collapsed);
}

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
		SkipCinematicButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCinematicSkipFinished);
	}
}

// Called when the current game state was changed
void UNMMCinematicStateWidget::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	// Show this widget in Cinematic state
	SetVisibility(CurrentGameState == ECurrentGameState::Cinematic ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	ResetWidget();
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

/*********************************************************************************************
 * Inputs
 ********************************************************************************************* */

// Is calling while the skip holding button is ongoing
void UNMMCinematicStateWidget::OnCinematicSkipOngoing_Implementation()
{
	const UWorld* World = GetWorld();
	checkf(World, TEXT("ERROR: [%i] %s:\n'World' is null!"), __LINE__, *FString(__FUNCTION__));
	const float NewHoldTime = CurrentHoldTimeInternal + World->GetDeltaSeconds();

	SetCurrentHoldTime(NewHoldTime);
}

// Is called on skip cinematic button released (cancelled)
void UNMMCinematicStateWidget::OnCinematicSkipReleased_Implementation()
{
	ResetWidget();
}

// Is called on the beginning of holding the skip button
void UNMMCinematicStateWidget::OnCinematicSkipStarted_Implementation()
{
	checkf(SkipText, TEXT("ERROR: [%i] %s:\n'SkipText' is null!"), __LINE__, *FString(__FUNCTION__));
	SkipText->SetVisibility(ESlateVisibility::Visible);
}

// Is called to skip cinematic on finished holding the skip button or clicked on UI
void UNMMCinematicStateWidget::OnCinematicSkipFinished_Implementation()
{
	// Skip cinematic
	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->SetGameStartingState();
	}
}
