// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Bomber.h"
//---
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

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "C++", meta = (DisplayName = "Set Game State"))
    void ServerSetGameState(ECurrentGameState NewGameState);

protected:
	/** Allows the PlayerController to set up custom input bindings. */
	virtual void SetupInputComponent() override;
};
