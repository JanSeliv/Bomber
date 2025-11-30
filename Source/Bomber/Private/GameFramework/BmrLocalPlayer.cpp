// Copyright (c) Yevhenii Selivanov

#include "GameFramework/BmrLocalPlayer.h"

// Bomber
#include "Controllers/BmrPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrLocalPlayer)

// Default constructor with overridden properties
UBmrLocalPlayer::UBmrLocalPlayer()
{
	// Ensures the correct PlayerController is spawned locally when the client joins a session
	PendingLevelPlayerControllerClass = ABmrPlayerController::StaticClass();
}