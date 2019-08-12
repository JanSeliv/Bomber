// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

/**
 * The HUD class
 */
UCLASS()
class BOMBER_API AMyHUD final : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Ñ++")
	class UUserWidget* UmgCurrentObj;

	/* Sets default values for this HUD's properties */
	AMyHUD();

	virtual void BeginPlay() final;
};
