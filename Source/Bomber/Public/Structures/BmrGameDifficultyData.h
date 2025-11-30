// Copyright (c) Yevhenii Selivanov

#pragma once

#include "BmrGameDifficultyData.generated.h"

/**
 * The type of the game difficulty.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBmrGameDifficulty : uint8
{
	None = 0 UMETA(Hidden),
	///< The easiest difficulty, is 0 as difficulty level
	Easy = 1 << 0,
	///< The normal difficulty, is 1 as difficulty level
	Normal = 1 << 1,
	///< The hardest difficulty, is 2 as difficulty level
	Hard = 1 << 2,
	///< Original game difficulty, where AI is hardcoded in controller, but very smart
	Vanilla = 1 << 3 UMETA(Hidden),
	Any = Easy | Normal | Hard | Vanilla
};

ENUM_CLASS_FLAGS(EBmrGameDifficulty);

/**
 * Determines the Game Features to be enabled by the game difficulty levels.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrDifficultyGameFeaturesData
{
	GENERATED_BODY()

	/** The name of the modular game feature that is enabled for the specified difficulties. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
	FName ModularGameFeatureName = NAME_None;

	/** Difficulty levels that are enabled for the specified game feature, multiple difficulties can be selected per each feature. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties, Bitmask, BitmaskEnum = "/Script/Bomber.EBmrGameDifficulty"))
	int32 GameDifficulties = 0;
};