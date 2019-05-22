// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Bomber.h"
#include "MyHUD.h"
#include "InGameUserWidget.h"

AMyPlayerController::AMyPlayerController()
{
	//use level 2D-camera withous switches
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
	InputComponent->BindAction<FEscapeDelegate>("EscapeEvent", IE_Pressed, Cast<UInGameUserWidget>(MyCustomHUD->umgCurrentObj), &UInGameUserWidget::ShowInGameState, NewObject<UObject>());
}


