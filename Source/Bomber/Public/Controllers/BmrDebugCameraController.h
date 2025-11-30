// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DebugCameraController.h"

#include "BmrDebugCameraController.generated.h"

/**
 * Is inherited from DebugCameraController to add custom functionality.
 */
UCLASS()
class BOMBER_API ABmrDebugCameraController : public ADebugCameraController
{
	GENERATED_BODY()

protected:
	/** Sets default values for this controller's properties. */
	ABmrDebugCameraController();

	/** Is overridden to prevent spawning the Debug HUD. */
	virtual void PostInitializeComponents() override;
};