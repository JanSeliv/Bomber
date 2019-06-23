// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/World.h"  //GetWorld()
#include "Kismet/GameplayStatics.h"

#define UE_LOG_STR(message, obj) UE_LOG(LogTemp, Warning, TEXT(message), *obj->GetName())

#define IS_TRANSIENT(obj) (obj->HasAllFlags(RF_Transient) || (UGameplayStatics::GetCurrentLevelName(obj->GetWorld()) == "Transient"))
#define IS_VALID(obj) ((obj != nullptr) && IsValid(obj) && !(obj)->IsPendingKill() && (obj)->IsValidLowLevel() && !IS_TRANSIENT(obj))
