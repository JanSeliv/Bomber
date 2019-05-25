// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT(" "));
#define PRINT(string) GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, string, true, FVector2D(2,2));
#include "MyGameModeBase.h"
#include "MyPlayerController.h"
#include "GeneratedMap.h"
#include "SingletonLibrary.h"
#include "Engine/World.h"//GetWorld()
#include "UObject/ConstructorHelpers.h"//ConstructorHelpers::FObjectFinder<UBlueprint> Blueprint_Effect_Fire()
#include "Kismet/GameplayStatics.h" // UGameplayStatics::
#include "TimerManager.h" // FTimerHandle timerHandle; 
#include "GameFramework/Character.h" //UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
