// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine\World.h"
#include "Kismet/GameplayStatics.h"

//@todo Is includes the transient world as IsEditorWorld() ? //!(Obj)->GetWorld()->IsGameWorld()
#define IS_TRANSIENT(Obj) (!(Obj)->IsValidLowLevelFast() || (Obj)->HasAllFlags(RF_Transient) || (Obj)->GetWorld() == nullptr || UGameplayStatics::GetCurrentLevelName((Obj)->GetWorld()) == "Transient")
#define IS_VALID(Obj) (IsValid(Obj) && !(Obj)->IsPendingKillPending() && !IS_TRANSIENT(Obj))

#define TO_FLAG(Enum) static_cast<int32>(Enum)

/**
 * Cell's types of danger
 * Breaks during cells searching on each side
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
 * Types of all actors on the Level Map
 * Where Walls, Boxes and Bombs are the physical barriers for players
 * It is possible to make a bitmask of actors types
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActorTypeEnum : uint8
{
	None = 0,								 ///< None of the types for comparisons
	Bomb = 1 << 0,							 ///< A destroyable exploding Obstacle
	Item = 1 << 1,							 ///< A picked element giving power-up (FPowerUp struct)
	Player = 1 << 2,						 ///< A character that is controlled by a person or bot
	Wall = 1 << 3,							 ///< An absolute static and unchangeable block throughout the game
	Box = 1 << 4,							 ///< A destroyable Obstacle
	All = Bomb | Item | Player | Wall | Box  ///< All level actors
};

/** Using EActorTypeEnum as a bitmask */
inline EActorTypeEnum operator|(const EActorTypeEnum& LType, const EActorTypeEnum& RType)
{
	return static_cast<EActorTypeEnum>(TO_FLAG(LType) | TO_FLAG(RType));
}

/** Checks the actors types among each other between themselves
 * @see USingletonLibrary::BitwiseActorTypes: blueprint analog */
inline bool operator&(const EActorTypeEnum& LType, const EActorTypeEnum& RType)
{
	return (TO_FLAG(LType) & TO_FLAG(RType)) != 0;
}

/**
 * Types of items
 */
UENUM(BlueprintType)
enum class EItemTypeEnum : uint8
{
	None,
	Skate,  ///< Increases speed
	Bomb,   ///< increases the amount of bombs
	Fire	///< Increases the range of explosion
};