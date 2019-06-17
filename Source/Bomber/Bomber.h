// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"			 //GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT(" "));
#include "Engine/World.h"			 //GetWorld()
#include "Kismet/GameplayStatics.h"  // UGameplayStatics::
#include "MapComponent.h"
#include "MyGameModeBase.h"
#include "MyPlayerController.h"
#include "SingletonLibrary.h"
#include "TimerManager.h"				 // FTimerHandle timerHandle;
#include "UObject/ConstructorHelpers.h"  //ConstructorHelpers::FObjectFinder<UBlueprint> Blueprint_Effect_Fire()

#define UE_LOG_STR(message, string) UE_LOG(LogTemp, Warning, TEXT(message), string)
#define ISVALID(obj) ((obj != nullptr) && IsValid(obj) && !(obj)->IsPendingKill() && (obj)->IsValidLowLevel())
#define ISTRANSIENT(obj) (obj->HasAllFlags(RF_Transient) || UGameplayStatics::GetCurrentLevelName(obj->GetWorld()) == "Transient")