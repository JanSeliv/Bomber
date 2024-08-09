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