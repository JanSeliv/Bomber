// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameModeBase.h"

#include "Bomber.h"
#include "MyHUD.h"
#include "MyPlayerController.h"

AMyGameModeBase::AMyGameModeBase()
{
	//tell custom game mode to use custom defaults
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = 0;
	HUDClass = nullptr;  //AMyHUD::StaticClass();
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMyGameModeBase::PossessController(AController* controller, APawn* pawn) const
{
	controller->Possess(pawn);

	//focus to game
	Cast<APlayerController>(controller)->SetInputMode(FInputModeGameOnly());
}
