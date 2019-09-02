// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"

#include "InGameUserWidget.h"
#include "MyHUD.h"

AMyPlayerController::AMyPlayerController()
{
	// Set this controller to call the Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;
}

//  Allows the PlayerController to set up custom input bindings
void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Focus to the game
	SetInputMode(FInputModeGameOnly());

	const auto MyCustomHUD = GetWorld()->SpawnActor<AMyHUD>(AMyHUD::StaticClass());
	if (MyCustomHUD)
	{
		//Call UInGameUserWidget::ShowInGameState function when the escape was pressed
		InputComponent->BindAction("EscapeEvent", IE_Pressed, Cast<UInGameUserWidget>(MyCustomHUD->UmgCurrentObj), &UInGameUserWidget::ShowInGameState);
	}
}