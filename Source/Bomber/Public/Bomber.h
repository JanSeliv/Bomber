// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Kismet/GameplayStatics.h"
#include "Bomber.generated.h"

/** IS_TRANSIENT returns true is specified object is pending kill, CDO or exists on the Transient level. */
static const FString TransientLevelName = TEXT("Transient");
#define IS_TRANSIENT(Obj) \
	( \
		!IsValid(Obj) \
		|| !(Obj)->IsValidLowLevelFast() \
		|| (Obj)->HasAllFlags(RF_ClassDefaultObject) \
		|| UGameplayStatics::GetCurrentLevelName(Obj) == TransientLevelName \
	)

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
