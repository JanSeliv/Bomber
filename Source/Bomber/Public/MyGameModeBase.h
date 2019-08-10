// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

UCLASS()
class BOMBER_API AMyGameModeBase final : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	void BeginPlay() final;

	UFUNCTION(BlueprintCallable, Category = "C++")
	void PossessController(AController* Controller, APawn* Pawn) const;
};