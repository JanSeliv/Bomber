﻿// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Engine/DataTable.h"
//---
#include "Bomber.h" // ELevelType
#include "Structures/PlayerTag.h"
//---
#include "NMMTypes.generated.h"

/**
 * Represents a row in the Cinematics Data Table.
 */
USTRUCT(BlueprintType)
struct NEWMAINMENU_API FNMMCinematicRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The row that does not contain any data. */
	static const FNMMCinematicRow Empty;

	/** The level where this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELevelType LevelType = ELT::None;

	/** The player for which this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerTag PlayerTag = FPlayerTag::None;

	/** The level sequence asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class ULevelSequence> LevelSequence = nullptr;

	/** The index of the subsequence, is taken from data table's row index on loading the LevelSequence. */
	UPROPERTY(BlueprintReadWrite)
	int32 RowIndex = INDEX_NONE;

	/** Returns true if this row is valid. */
	bool IsValid() const;

	/** Equal operator. */
	bool operator==(const FNMMCinematicRow& Other) const;

	/** Returns true is this does not contain any data. */
	bool FORCEINLINE IsEmpty() const { return *this == Empty; }
};

/**
 * Represents the state of the Main Menu cinematics.
 */
UENUM(BlueprintType, DisplayName = "New Main Menu State")
enum class ENMMState : uint8
{
	///< Is not in the Menu
	None,
	///< Player has camera moving between Spots, both spots are playing idle in loop
	Transition,
	///< Player sees chosen character, only this spot is playing idle in loop
	Idle,
	///< Is playing the main parts of cinematic after play button is pressed
	Cinematic
};

/**
 * Represents the state of the camera rail state.
 */
UENUM(BlueprintType, DisplayName = "New Main Menu Camera Rail State")
enum class ENMMCameraRailTransitionState : uint8
{
	///< Transition is not started
	None,
	///< Camera moving between Spots started
	BeginTransition,
	///< Camera moving between Spots reached it's halfway
	HalfwayTransition,
	///< Camera moving between Spots finished transition
	EndTransition
};
