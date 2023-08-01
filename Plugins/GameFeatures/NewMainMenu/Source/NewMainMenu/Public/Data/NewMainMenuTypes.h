// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Engine/DataTable.h"
//---
#include "Bomber.h" // ELevelType
#include "DataAssets/PlayerDataAsset.h" // FPlayerTag
//---
#include "NewMainMenuTypes.generated.h"

/**
 * Represents a row in the Cinematics Data Table.
 */
USTRUCT(BlueprintType)
struct NEWMAINMENU_API FCinematicRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The level where this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELevelType LevelType = ELT::None;

	/** The player for which this cinematic should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerTag PlayerTag = FPlayerTag::None;

	/** The level sequence asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class ULevelSequence> LevelSequence = nullptr;
};
