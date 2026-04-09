// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "Bomber.h" // EBmrLevelType
#include "DalRegistryRow.h"
#include "Structures/BmrPlayerTag.h"

#include "BmrCinematicRow.generated.h"

/**
 * Row struct for cinematics data table registered via Data Registry.
 * Mods or maps register their own data tables with cinematic rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrCinematicRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrCinematicRow>
#endif
{
	GENERATED_BODY()

	/** The row that does not contain any data. */
	static const FBmrCinematicRow Empty;

	/** The level where this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBmrLevelType LevelType = ELT::None;

	/** The player for which this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** The level sequence asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class ULevelSequence> LevelSequence = nullptr;

	/** User-defined priority that determines cinematic play order and spot selection, higher value = higher priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = INDEX_NONE;

	/** Returns true if this row is valid. */
	bool IsValid() const;

	/** Equal operator. */
	bool operator==(const FBmrCinematicRow& Other) const;

	/** Returns true is this does not contain any data. */
	bool FORCEINLINE IsEmpty() const { return *this == Empty; }
};