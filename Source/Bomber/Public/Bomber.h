// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/GameplayStatics.h"
#include "Bomber.generated.h"

#define IS_TRANSIENT(Obj) (!(Obj) || !(Obj)->IsValidLowLevelFast() || (Obj)->HasAnyFlags(RF_Transient | RF_ClassDefaultObject) || (Obj)->GetWorld() == nullptr || UGameplayStatics::GetCurrentLevelName((Obj)->GetWorld()) == "Transient")
#define IS_VALID(Obj) (IsValid(Obj) && !(Obj)->IsPendingKillPending() && !IS_TRANSIENT(Obj))

#define TO_FLAG(Enum) static_cast<int32>(Enum)
#define TO_ENUM(Enum, Bitmask) static_cast<Enum>(Bitmask)

#define ECC_Player0 ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Player1 ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Player2 ECollisionChannel::ECC_GameTraceChannel3
#define ECC_Player3 ECollisionChannel::ECC_GameTraceChannel4
#define ECC_UI ECollisionChannel::ECC_GameTraceChannel5

#define TEXT_NONE FCoreTexts::Get().None

/**
* Types of all actors on the Level Map
* Where Walls, Boxes and Bombs are the physical barriers for players
* It is possible to make a bitmask of actors types
*/
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActorType : uint8
{
	///< None of the types for comparisons
	None = 0,
	///< A destroyable exploding Obstacle
	Bomb = 1 << 0,
	///< A destroyable Obstacle
	Box = 1 << 1,
	///< A picked element giving power-up (FPowerUp struct)
	Item = 1 << 2,
	///< A character that is controlled by a person or bot
	Player = 1 << 3,
	///< An absolute static and unchangeable block throughout the game
	Wall = 1 << 4,
	///< All actor types
	All = Bomb | Item | Player | Wall | Box
};

ENUM_CLASS_FLAGS(EActorType);
using EAT = EActorType;

/**
 * Levels in the game. In some cases bitmask are used to filter meshes.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ELevelType : uint8
{
	None = 0,
	First = 1 << 0 UMETA(DisplayName = "Maya"),
	Second = 1 << 1 UMETA(DisplayName = "City"),
	Third = 1 << 2 UMETA(DisplayName = "Forest"),
	Fourth = 1 << 3 UMETA(DisplayName = "Water"),
	Max = First | Second | Third | Fourth UMETA(DisplayName = "Any") ///< Can be used for such levels as menu, sandbox, etc.
};

ENUM_CLASS_FLAGS(ELevelType);
using ELT = ELevelType;
#define ELT_FIRST_FLAG TO_FLAG(ELT::First)
#define ELT_LAST_FLAG TO_FLAG(ELT::Fourth)

/**
 * Pathfinding types by which cells could be found.
 */
UENUM(BlueprintType)
enum class EPathType : uint8
{
	///< Break by the first AT::Wall without obstacles
	Explosion,
	///< Break by the first AT::Wall + EAT::Bomb + EAT::Box
	Free,
	///< Break by the first AT::Wall + EAT::Bomb + EAT::Box + explosions
	Safe,
	///< Break by the first AT::Wall + EAT::Bomb + EAT::Box + explosions + AT::Player
	Secure,
	///< Do not break the path
	Any
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
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECurrentGameState : uint8
{
	None = 0,
	Menu = 1 << 0,
	GameStarting = 1 << 1,
	EndGame = 1 << 2,
	InGame = 1 << 3,
	Max = Menu | GameStarting | EndGame | InGame
};

ENUM_CLASS_FLAGS(ECurrentGameState);
using ECGS = ECurrentGameState;

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
