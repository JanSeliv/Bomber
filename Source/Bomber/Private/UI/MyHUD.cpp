// Copyright 2020 Yevhenii Selivanov.

#include "MyHUD.h"
//---
#include "Bomber.h"
#include "MyGameStateBase.h"
#include "SingletonLibrary.h"
//---
#include "ConstructorHelpers.h"
#include "UserWidget.h"

// Sets default values for this HUD's properties
AMyHUD::AMyHUD()
{
	// Find In Game User Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> UmgLevelClassFinder(TEXT("/Game/Bomber/UI/InGame/WBP_InGame"));
	if (UmgLevelClassFinder.Succeeded())
	{
		InGameWidgetClass = UmgLevelClassFinder.Class;
	}
}


// Called when the game starts. Created widget.
void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	// Widget creating and adding it to viewport
	InGameWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), InGameWidgetClass);
	if (InGameWidget) // successfully created
	{
		InGameWidget->SetVisibility(ESlateVisibility::Hidden);
		InGameWidget->AddToViewport();
	}

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStarted);
	}
}

void AMyHUD::OnGameStarted(ECurrentGameState CurrentGameState)
{
	if(!InGameWidget)
	{
		return;
	}

	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		{
			InGameWidget->SetVisibility(ESlateVisibility::Hidden);
			break;
		}
		case ECurrentGameState::GameStarting:
		{
			InGameWidget->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	}
}
