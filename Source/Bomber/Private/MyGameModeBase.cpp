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

	//focus to the game
	const auto PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerController)
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}
