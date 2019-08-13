// Fill out your copyright notice in the Description page of Project Settings.

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
	/** Sets default values for this character's properties */
	AMyPlayerController();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Allows the PlayerController to set up custom input bindings. */
	virtual void SetupInputComponent() final;
};
