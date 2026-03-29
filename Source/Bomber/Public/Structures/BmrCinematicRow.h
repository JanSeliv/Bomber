// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "Bomber.h" // EBmrLevelType
#include "Structures/BmrPlayerTag.h"

#include "BmrCinematicRow.generated.h"

/**
 * Row struct for cinematics data table registered via Data Registry.
 * Map MGFs register their own data tables with cinematic rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrCinematicRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The row that does not contain any data. */
	static const FBmrCinematicRow Empty;

	/** The name of the DR_Cinematics Data Registry type used to query cinematic rows. */
	static inline const FName CinematicsRegistryTypeName = TEXT("DR_Cinematics");

	/** The level where this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBmrLevelType LevelType = ELT::None;

	/** The player for which this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** The level sequence asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class ULevelSequence> LevelSequence = nullptr;

	/** The index of the subsequence, is taken from data table's row index on loading the LevelSequence. */
	UPROPERTY(BlueprintReadWrite)
	int32 RowIndex = INDEX_NONE;

	/** Returns true if this row is valid. */
	bool IsValid() const;

	/** Equal operator. */
	bool operator==(const FBmrCinematicRow& Other) const;

	/** Returns true is this does not contain any data. */
	bool FORCEINLINE IsEmpty() const { return *this == Empty; }
};