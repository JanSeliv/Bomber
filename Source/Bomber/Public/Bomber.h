// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Bomber.generated.h"

#define IS_TRANSIENT(Obj) (FTransientChecker::IsTransient(Obj))

namespace FTransientChecker
{
	/** Returns true is specified object is pending kill, CDO or exists on the Transient level. */
	BOMBER_API bool IsTransient(const UObject* Obj);
	BOMBER_API bool IsTransientLevel(const UObject* Obj);
} // namespace FTransientChecker

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
enum class EBmrActorType : uint8
{
	///< None of the types for comparisons
	None = 0,
	///< A destroyable exploding Obstacle
	Bomb = 1 << 0,
	///< A destroyable Obstacle
	Box = 1 << 1,
	///< A picked element giving power-up (FPowerUp struct)
	Powerup = 1 << 2,
	///< A character that is controlled by a person or bot
	Player = 1 << 3,
	///< An absolute static and unchangeable block throughout the game
	Wall = 1 << 4,
	///< All actor types
	All = Bomb | Powerup | Player | Wall | Box
};

ENUM_CLASS_FLAGS(EBmrActorType);
using EAT = EBmrActorType;

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
 * The round result.
 * Can be tracked by listening ABmrPlayerState::OnEndGameStateChanged delegate.
 */
UENUM(BlueprintType)
enum class EBmrEndGameState : uint8
{
	None,
	///< Player is last alive
	Win,
	///< Player has died
	Lose,
	///< Player has killed anyone else before dying
	HonorLoss,
	///< Last players have blasted each other or the time has run out
	Draw
};

/**
 * Represents the type of the character.
 */
UENUM(BlueprintType)
enum class EBmrPlayerType : uint8
{
	None,
	Human,
	Bot,
	Any
};