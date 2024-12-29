// Copyright (c) Yevhenii Selivanov

#include "Controllers/MyDebugCameraController.h"
//---
#include "GameFramework/MyCheatManager.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MyDebugCameraController)

// Default constructor
AMyDebugCameraController::AMyDebugCameraController()
{
	// Is overridden to have the same cheat manager as the player controller, so UMyCheatManager::DisableDebugCamera is called
	CheatClass = UMyCheatManager::StaticClass();
}

// Is overridden to prevent spawning the Debug HUD
void AMyDebugCameraController::PostInitializeComponents()
{
	// Don't call Super::PostInitializeComponents to prevent spawning the Debug HUD,
	// but still call the APlayerController's method to finish controller initialization
	APlayerController::PostInitializeComponents();

	ChangeState(NAME_Inactive);
}