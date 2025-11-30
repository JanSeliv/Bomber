// Copyright (c) Yevhenii Selivanov

#include "Controllers/BmrDebugCameraController.h"

// Bomber
#include "GameFramework/BmrCheatManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrDebugCameraController)

// Default constructor
ABmrDebugCameraController::ABmrDebugCameraController()
{
	// Is overridden to have the same cheat manager as the player controller, so UBmrCheatManager::DisableDebugCamera is called
	CheatClass = UBmrCheatManager::StaticClass();
}

// Is overridden to prevent spawning the Debug HUD
void ABmrDebugCameraController::PostInitializeComponents()
{
	// Don't call Super::PostInitializeComponents to prevent spawning the Debug HUD,
	// but still call the APlayerController's method to finish controller initialization
	APlayerController::PostInitializeComponents();

	ChangeState(NAME_Inactive);
}