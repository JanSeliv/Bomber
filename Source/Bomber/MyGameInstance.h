// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 *
 */
UCLASS()
class BOMBER_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	void Init() override {
		Super::Init();

		// Start MenuLevel
		//UGameplayStatics::OpenLevel(this, "/Game/Bomber/GameLevels/MenuLevel", false);
	}
};
