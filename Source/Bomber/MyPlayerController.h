// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

UCLASS()
class BOMBER_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void testfun(int a, bool b);
	AMyPlayerController();

	void BeginPlay() override;

	void SetupInputComponent() override;

	//widgets
	UPROPERTY(BlueprintReadWrite, Category = "C++")
		class AMyHUD* MyCustomHUD;
};
