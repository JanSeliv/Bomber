// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameModeBase.h"

#include "Bomber.h"
#include "MyPlayerController.h"

AMyGameModeBase::AMyGameModeBase()
{
	//tell custom game mode to use custom defaults
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;  //AMyHUD::StaticClass();
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMyGameModeBase::PossessController(AController* Controller, APawn* Pawn) const
{
	Controller->Possess(Pawn);

	//focus to game
	Cast<APlayerController>(Controller)->SetInputMode(FInputModeGameOnly());
}
