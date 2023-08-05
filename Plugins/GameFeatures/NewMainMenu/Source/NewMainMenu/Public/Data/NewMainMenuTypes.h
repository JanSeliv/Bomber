// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Engine/DataTable.h"
//---
#include "Bomber.h" // ELevelType
#include "Structures/PlayerTag.h"
//---
#include "NewMainMenuTypes.generated.h"

/**
 * Represents a row in the Cinematics Data Table.
 */
USTRUCT(BlueprintType)
struct NEWMAINMENU_API FCinematicRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The row that does not contain any data. */
	static const FCinematicRow Empty;

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
	bool operator==(const FCinematicRow& Other) const;

	/** Returns true is this does not contain any data. */
	bool FORCEINLINE IsEmpty() const { return *this == Empty; }
};
