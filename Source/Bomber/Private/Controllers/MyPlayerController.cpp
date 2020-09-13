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

	// Focus to the game
	SetInputMode(FInputModeGameOnly());

	// Call UInGameWidget::ShowInGameState function when the escape was pressed
	FInputActionBinding EscapeWasPressedLambda("EscapeEvent", IE_Pressed);
	EscapeWasPressedLambda.ActionDelegate.GetDelegateForManualSet().BindLambda([this]() {
		const auto MyCustomHUD = Cast<AMyHUD>(GetHUD());
		const auto InGameWidget = MyCustomHUD ? Cast<UInGameWidget>(MyCustomHUD->InGameWidget) : nullptr;
		if (InGameWidget)
		{
			InGameWidget->ShowInGameState();
		}
	});
	InputComponent->AddActionBinding(EscapeWasPressedLambda);
}
