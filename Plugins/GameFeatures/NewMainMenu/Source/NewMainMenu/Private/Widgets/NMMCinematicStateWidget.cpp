// Copyright (c) Yevhenii Selivanov

#include "Widgets/NMMCinematicStateWidget.h"
//---
#include "Bomber.h"
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMSubsystem.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCinematicStateWidget)

// Is called to skip cinematic
void UNMMCinematicStateWidget::SkipCinematic()
{
	if (UNMMSpotComponent* ActiveSpot = UNMMSubsystem::Get().GetActiveMainMenuSpotComponent())
	{
		ActiveSpot->StopMasterSequence();
	}

	if (UMyCameraComponent* CameraComponent = UMyBlueprintFunctionLibrary::GetLevelCamera())
	{
		constexpr bool bBlend = false;
		CameraComponent->PossessCamera(bBlend);
	}

	if (AMyPlayerController* MyPC = GetOwningPlayer<AMyPlayerController>())
	{
		MyPC->SetGameStartingState();
	}
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
