// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

//@todo Is includes the transient world as IsEditorWorld() ? //!(Obj)->GetWorld()->IsGameWorld()
#define IS_TRANSIENT(Obj) (!(Obj) || !(Obj)->IsValidLowLevelFast() || (Obj)->HasAllFlags(RF_Transient) || (Obj)->GetWorld() == nullptr || UGameplayStatics::GetCurrentLevelName((Obj)->GetWorld()) == "Transient")
#define IS_VALID(Obj) (IsValid(Obj) && !(Obj)->IsPendingKillPending() && !IS_TRANSIENT(Obj))

#define TO_FLAG(Enum) static_cast<int32>(Enum)
#define TO_ENUM(Enum, Bitmask) static_cast<Enum>(Bitmask)

/**
* Types of all actors on the Level Map
* Where Walls, Boxes and Bombs are the physical barriers for players
* It is possible to make a bitmask of actors types
*/
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActorType : uint8
{
	None = 0,
	///< None of the types for comparisons
	Bomb = 1 << 0,
	///< A destroyable exploding Obstacle
	Box = 1 << 1,
	///< A destroyable Obstacle
	Item = 1 << 2,
	///< A picked element giving power-up (FPowerUp struct)
	Player = 1 << 3,
	///< A character that is controlled by a person or bot
	Wall = 1 << 4,
	///< An absolute static and unchangeable block throughout the game
	All = Bomb | Item | Player | Wall | Box ///< All actor types
};

ENUM_CLASS_FLAGS(EActorType);
using EAT = EActorType;

/**
 * Levels in the game. In some cases bitmask are used to filter meshes.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ELevelType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	First = 1 << 0 UMETA(DisplayName = "Maya"),
	Second = 1 << 1 UMETA(DisplayName = "City"),
	Third = 1 << 2 UMETA(DisplayName = "Forest"),
	Fourth = 1 << 3 UMETA(DisplayName = "Water"),
	Max = First | Second | Third | Fourth UMETA(DisplayName = "Any") ///< Can be used for such levels as menu, sandbox, etc.
};

ENUM_CLASS_FLAGS(ELevelType);
using ELT = ELevelType;

/**
 * Pathfinding types by danger extents.
 */
UENUM(BlueprintType)
enum class EPathType : uint8
{
	Explosion,
	///< Break to the first AT::Wall without obstacles
	Free,
	///< Break to the first AT::Wall + obstacles
	Safe,
	///< Break to the first AT::Wall + obstacles + explosions
	Secure ///< Break to the first AT::Wall + obstacles + explosions + AT::Player
};

/**
 * Types of items.
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	///< The type was not selected
	Skate,
	///< Increases speed
	Bomb,
	///< increases the amount of bombs
	Fire ///< Increases the range of explosion
};

/**
 * The replicated states of the game.
 */
UENUM(BlueprintType)
enum class ECurrentGameState : uint8
{
	None,
	Menu,
	GameStarting,
	EndGame,
	InGame
};

/**
 * The round result.
 */
UENUM(BlueprintType)
enum class EEndGameState : uint8
{
	None,
	Win,
	Lose,
	Draw
};
