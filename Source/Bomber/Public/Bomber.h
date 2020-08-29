// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Engine\World.h"
#include "Kismet/GameplayStatics.h"

//@todo Is includes the transient world as IsEditorWorld() ? //!(Obj)->GetWorld()->IsGameWorld()
#define IS_TRANSIENT(Obj) (!(Obj) || !(Obj)->IsValidLowLevelFast() || (Obj)->HasAllFlags(RF_Transient) || (Obj)->GetWorld() == nullptr || UGameplayStatics::GetCurrentLevelName((Obj)->GetWorld()) == "Transient")
#define IS_VALID(Obj) (IsValid(Obj) && !(Obj)->IsPendingKillPending() && !IS_TRANSIENT(Obj))

#define TO_FLAG(Enum) static_cast<int32>(Enum)

/**
 * Pathfinding types by danger extents.
 */
UENUM(BlueprintType)
enum class EPathType : uint8
{
	Explosion,	///< Break to the first EActorType::Wall without obstacles
	Free,		///< Break to the first EActorType::Wall + obstacles
	Safe,		///< Break to the first EActorType::Wall + obstacles + explosions
	Secure		///< Break to the first EActorType::Wall + obstacles + explosions + EActorType::Player
};

/**
* Levels in the game.
*/
UENUM(BlueprintType)
enum class ELevelType : uint8
{
	LT_First 	UMETA(DisplayName = "Maya"),
	LT_Second 	UMETA(DisplayName = "City"),
	LT_Third  	UMETA(DisplayName = "Forest"),
	LT_Max	  	UMETA(DisplayName = "Any")  ///< Can be used for such levels as menu, sandbox or any, keep as LAST enum
};
ENUM_RANGE_BY_COUNT(ELevelType, ELevelType::LT_Max);

/**
 * Types of all actors on the Level Map
 * Where Walls, Boxes and Bombs are the physical barriers for players
 * It is possible to make a bitmask of actors types
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActorType : uint8
{
	None = 0,								 ///< None of the types for comparisons
	Bomb = 1 << 0,							 ///< A destroyable exploding Obstacle
	Box = 1 << 1,							 ///< A destroyable Obstacle
	Item = 1 << 2,							 ///< A picked element giving power-up (FPowerUp struct)
	Player = 1 << 3,						 ///< A character that is controlled by a person or bot
	Wall = 1 << 4,							 ///< An absolute static and unchangeable block throughout the game
	All = Bomb | Item | Player | Wall | Box	 ///< All level actors
};

/** Bitwise OR operator for setting a bitmask of actors types. */
FORCEINLINE EActorType operator|(const EActorType& LType, const EActorType& RType)
{
	return static_cast<EActorType>(TO_FLAG(LType) | TO_FLAG(RType));
}

/** Bitwise NOT operator for turning off actors types. */
FORCEINLINE EActorType operator~(const EActorType& RType)
{
	return static_cast<EActorType>(TO_FLAG(EActorType::All) & ~TO_FLAG(RType));
}

/** Checks the actors types among each other between themselves.
 * @see USingletonLibrary::BitwiseActorTypes: blueprint analog. */
FORCEINLINE bool operator&(const EActorType& LType, const int32& Bitmask)
{
	return (TO_FLAG(LType) & Bitmask) != 0;
}

/**
 * Types of items.
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,	///< The type was not selected
	Skate,	///< Increases speed
	Bomb,	///< increases the amount of bombs
	Fire	///< Increases the range of explosion
};
