// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine\World.h"
#include "Kismet/GameplayStatics.h"

#define UE_LOG_STR(UObj, FunctionChars, String) UE_LOG(LogTemp, Warning, TEXT("\t %s \t %s \t %s"), *UObj->GetName(), *FString(FunctionChars), *FString(String))

#define IS_TRANSIENT(Obj) ((Obj->HasAllFlags(RF_Transient) || (Obj->GetWorld() == nullptr) || (UGameplayStatics::GetCurrentLevelName(Obj->GetWorld()) == "Transient")))
#define IS_VALID(Obj) (IsValid(Obj) && (Obj)->IsValidLowLevel() && !IS_TRANSIENT(Obj))

#define IS_PIE(World) (ensureMsgf(World != nullptr, TEXT("World is null")) && World->HasBegunPlay() == false && (World->WorldType == EWorldType::Editor))

/**
 * @defgroup path_types Receiving cells for their type of danger
 * Types of breaks during cells searching on each side
 */
UENUM(BlueprintType)
enum class EPathTypesEnum : uint8
{
	Explosion,  ///< Break to the first EActorTypeEnum::Wall without obstacles
	Free,		///< Break to the first EActorTypeEnum::WallWall + obstacles
	Safe,		///< Break to the first EActorTypeEnum::WallWall + obstacles + explosions
	Secure		///< Break to the first EActorTypeEnum::WallWall + obstacles + explosions + EActorTypeEnum::Player
};

/**
 * @defgroup actor_types Group where used types of actors
 * Types of all actors on the Level Map
 * Where Walls, Boxes and Bombs are the physical barrier for players
 */
UENUM(BlueprintType, meta = (Bitflags))
enum class EActorTypeEnum : uint8
{
	None = 0,		  ///< None of the types for comparisons
	Bomb = 1 << 0,	///< A destroyable exploding Obstacle
	Item = 1 << 1,	///< A picked element giving power-up (FPowerUp struct)
	Player = 1 << 2,  ///< A character that is controlled by a person or bot
	Wall = 1 << 3,	///< An absolute static and unchangeable block throughout the game
	Box = 1 << 4	  ///< A destroyable Obstacle
};

/**
 * @defgroup item_types Group where used types of items
 * Types of items
 */
UENUM(BlueprintType)
enum class EItemTypeEnum : uint8
{
	None = 0,
	Skate = 1 << 0,  ///< Increases speed
	Bomb = 1 << 1,   ///< increases the amount of bombs
	Fire = 1 << 2	///< Increases the range of explosion
};