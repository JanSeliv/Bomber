// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Bomber.generated.h"

#define IS_TRANSIENT(Obj) ( FTransientChecker::IsTransient(Obj) )

namespace FTransientChecker
{
/** Returns true is specified object is pending kill, CDO or exists on the Transient level. */
BOMBER_API bool IsTransient(const UObject* Obj);
}

/**
 * Is useful for work with bit flags.
 */
#define TO_FLAG(Enum) static_cast<int32>(Enum)
#define TO_ENUM(Enum, Bitmask) static_cast<Enum>(Bitmask)

/**
 * Custom collision channels.
 */
#define ECC_Player0 ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Player1 ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Player2 ECollisionChannel::ECC_GameTraceChannel3
#define ECC_Player3 ECollisionChannel::ECC_GameTraceChannel4
#define ECC_UI ECollisionChannel::ECC_GameTraceChannel5

/** Is init version of TEXT("None"). */
#define TEXT_NONE FCoreTexts::Get().None

/** Define Bomber log category. */
BOMBER_API DECLARE_LOG_CATEGORY_EXTERN(LogBomber, Log, All);

/**
* Types of all actors on the Generated Map
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
 * Levels in the game.
 * In many cases is used to get the specific mesh of an level actor by the level type.
 * @see ULevelActorRow::LevelType
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ELevelType : uint8
{
	None = 0,
	///< Represents Maya level
	First = 1 << 0 UMETA(DisplayName = "Maya"),
	///< Represents City level
	Second = 1 << 1 UMETA(DisplayName = "City"),
	///< Represents Forest level
	Third = 1 << 2 UMETA(DisplayName = "Forest"),
	///< Represents Water level
	Fourth = 1 << 3 UMETA(DisplayName = "Water"),
	///< All the types, also can be used for such levels as menu, sandbox, etc.
	Max = First | Second | Third | Fourth UMETA(DisplayName = "Any")
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
	///< The type was not selected
	None,
	///< Increases speed
	Skate,
	///< increases the amount of bombs
	Bomb,
	///< Increases the range of explosion
	Fire
};

using EIT = EItemType;
#define EIT_FIRST_FLAG TO_FLAG(EIT::Skate)
#define EIT_LAST_FLAG TO_FLAG(EIT::Fire)

/**
 * The replicated states of the game. It shares the state between all the players at the same time.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECurrentGameState : uint8
{
	None = 0,
	///< Is active while players are in Main-Menu.
	Menu = 1 << 0,
	///< Is active while players see count-down time (3-2-1).
	GameStarting = 1 << 1,
	///< Is active when the match is finished and players see their results of the game.
	EndGame = 1 << 2,
	///< Is active during the active match.
	InGame = 1 << 3,
	///< Is active while player is watching cutscene of chosen character, is happening in single-player only since cinematics are automatically skipped in multiplayer.
	Cinematic = 1 << 4,
	Max = Menu | GameStarting | EndGame | InGame | Cinematic UMETA(DisplayName = "Any")
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
