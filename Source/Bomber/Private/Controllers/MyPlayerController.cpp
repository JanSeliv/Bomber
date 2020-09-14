// Copyright 2020 Yevhenii Selivanov.

#include "MyPlayerController.h"
//---
#include "InGameWidget.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyHUD.h"
#include "SingletonLibrary.h"

// Sets default values for this controller's properties
AMyPlayerController::AMyPlayerController()
{
	// Set this controller to call the Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;
}

void AMyPlayerController::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	// Listen states to manage the tick
	if(AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->ServerSetGameState(NewGameState);
	}
}

bool AMyPlayerController::ServerSetGameState_Validate(ECurrentGameState NewGameState)
{
	return true;
}

//  Allows the PlayerController to set up custom input bindings
void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Listen states
	if(AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	SetIgnoreMoveInput(true);
}

// Locks or unlocks movement input
void AMyPlayerController::SetIgnoreMoveInput(bool bNewMoveInput)
{
	IgnoreMoveInput = bNewMoveInput;
	bShowMouseCursor = bNewMoveInput;
	bEnableClickEvents = bNewMoveInput;
	bEnableMouseOverEvents = bNewMoveInput;

	if(bNewMoveInput)
	{
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

//
void AMyPlayerController::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
        case ECurrentGameState::GameStarting:
		{
			SetIgnoreMoveInput(true);
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetIgnoreMoveInput(false);
			break;
		}
		default: break;
	}
}

