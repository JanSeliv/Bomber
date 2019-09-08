// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"

#include "MyPlayerController.generated.h"

/**
 * The player controller class
 */
UCLASS()
class BOMBER_API AMyPlayerController final : public APlayerController
{
	GENERATED_BODY()

public:
	/** Sets default values for this controller's properties. */
	AMyPlayerController();

protected:
	/** Allows the PlayerController to set up custom input bindings. */
	virtual void SetupInputComponent() override;
};
