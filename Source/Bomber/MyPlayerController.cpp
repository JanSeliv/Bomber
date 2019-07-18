// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "InGameUserWidget.h"
#include "MyHUD.h"

AMyPlayerController::AMyPlayerController()
{
	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;
}

void AMyPlayerController::BeginPlay()
{
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	MyCustomHUD = GetWorld()->SpawnActor<AMyHUD>(AMyHUD::StaticClass());
	//Call UInGameUserWidget::ShowInGameState function when the escape was pressed
	InputComponent->BindAction("EscapeEvent", IE_Pressed, Cast<UInGameUserWidget>(MyCustomHUD->UmgCurrentObj), &UInGameUserWidget::ShowInGameState);
}
