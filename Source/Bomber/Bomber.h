// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/World.h"  //GetWorld()
#include "Kismet/GameplayStatics.h"

#define UE_LOG_STR(message, Obj) UE_LOG(LogTemp, Warning, TEXT(message), *Obj->GetName())

#define IS_TRANSIENT(Obj) ((Obj->HasAllFlags(RF_Transient) || (Obj->GetWorld() == nullptr) || (UGameplayStatics::GetCurrentLevelName(Obj->GetWorld()) == "Transient")))
#define IS_VALID(Obj) ((Obj != nullptr) && IsValid(Obj) && !(Obj)->IsPendingKill() && (Obj)->IsValidLowLevel() && !IS_TRANSIENT(Obj))

#define IS_PIE(World) (World != nullptr && World->HasBegunPlay() == false)