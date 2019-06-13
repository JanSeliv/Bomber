// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

UCLASS()
class BOMBER_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "C++")
	void PossessController(AController* controller, APawn* pawn) const;
};