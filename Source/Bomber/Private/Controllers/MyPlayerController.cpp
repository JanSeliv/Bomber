// Copyright 2021 Yevhenii Selivanov.

#include "Controllers/MyPlayerController.h"
//---
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "UI/InGameWidget.h"
#include "UI/MyHUD.h"

// Sets default values for this controller's properties
AMyPlayerController::AMyPlayerController()
{
	// Set this controller to call the Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Use level 2D-camera without switches
	bAutoManageActiveCameraTarget = false;

	// Set cheat class
	CheatClass = UMyCheatManager::StaticClass();
}

// Set the new game state for the current game
void AMyPlayerController::ServerSetGameState_Implementation(ECurrentGameState NewGameState)
{
	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->ServerSetGameState(NewGameState);
	}
}

// Returns true if the mouse cursor can be hidden
bool AMyPlayerController::CanHideMouse() const
{
	switch (AMyGameStateBase::GetCurrentGameState(this))
	{
		case ECurrentGameState::GameStarting:
		case ECurrentGameState::InGame:
			return true;
		default:
			return false;
	}
}

// Called to to set mouse cursor visibility
void AMyPlayerController::SetMouseVisibility(bool bShouldShow)
{
	if (!bShouldShow
	    && !CanHideMouse())
	{
		return;
	}

	bShowMouseCursor = bShouldShow;
	bEnableClickEvents = bShouldShow;
	bEnableMouseOverEvents = bShouldShow;

	if (bShouldShow)
	{
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

// Go back input for UI widgets
void AMyPlayerController::GoUIBack()
{
	if (AMyHUD* HUD = USingletonLibrary::GetMyHUD())
	{
		HUD->GoUIBack();
	}
}

//  Allows the PlayerController to set up custom input bindings
void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!ensureMsgf(InputComponent, TEXT("ASSERT: 'InputComponent' is not valid")))
	{
		return;
	}

	// Do not consume added input
	auto SetInput = [](FInputBinding& InputRef) { InputRef.bConsumeInput = false; };

	static const FName GoUIBackName = GET_FUNCTION_NAME_CHECKED(AMyHUD, GoUIBack);
	SetInput(InputComponent->BindAction(GoUIBackName, IE_Pressed, this, &ThisClass::GoUIBack));
}

// Called when the game starts or when spawned
void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	OnGameStateChanged(ECurrentGameState::Menu);

	ExecuteDefaultConsoleCommands();
}

// Locks or unlocks movement input
void AMyPlayerController::SetIgnoreMoveInput(bool bShouldIgnore)
{
	// Do not call super to avoid stacking, override it

	if (!bShouldIgnore
	    && !CanHideMouse())
	{
		return;
	}

	SetMouseVisibility(bShouldIgnore);
	IgnoreMoveInput = bShouldIgnore;
}

// Called default console commands on begin play
void AMyPlayerController::ExecuteDefaultConsoleCommands()
{
	static const FString NaniteDisableCommand(TEXT("r.Nanite 0"));
	ConsoleCommand(NaniteDisableCommand);
}

// Listen to toggle movement input and mouse cursor
void AMyPlayerController::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			SetIgnoreMoveInput(true);
			SetMouseVisibility(false);
			break;
		}
		case ECurrentGameState::Menu:
		case ECurrentGameState::EndGame:
		{
			SetIgnoreMoveInput(true);
			break;
		}
		case ECurrentGameState::InGame:
		{
			SetIgnoreMoveInput(false);
			break;
		}
		default:
			break;
	}
}
