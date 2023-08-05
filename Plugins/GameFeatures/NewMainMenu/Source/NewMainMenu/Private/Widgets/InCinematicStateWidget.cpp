// Copyright (c) Yevhenii Selivanov

#include "Widgets/InCinematicStateWidget.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InCinematicStateWidget)

// // Called after the underlying slate widget is constructed
void UInCinematicStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
void UInCinematicStateWidget::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show this widget in Cinematic state
	SetVisibility(CurrentGameState == ECurrentGameState::Cinematic ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Is called to start listening game state changes
void UInCinematicStateWidget::BindOnGameStateChanged(AMyGameStateBase* MyGameState)
{
	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);
}
