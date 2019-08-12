// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 * The custom game mode class
 */
UCLASS()
class BOMBER_API AMyGameModeBase final : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Sets default values for this character's properties */
	AMyGameModeBase();

protected:
	/** Called when the game starts or when spawned */
	void BeginPlay() final;
};