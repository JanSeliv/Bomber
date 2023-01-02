// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
//---
#include "MorphsPlayerTypes.generated.h"

/**
 * A: start value
 * B: end value
 */
UENUM(BlueprintType)
enum class EPlaybackType : uint8
{
	/// Set a specified start value without playing
	None UMETA(DisplayName = "A"),
	/// Play from start to end
	FromStart UMETA(DisplayName = "A to B"),
	/// Play from end to start
	FromEnd UMETA(DisplayName = "B to A"),
	/// Play from start to end and back to start
	Max UMETA(DisplayName = "A to B to A"),
};

/**
 * Data about morph target (shape key) to be played.
 */
USTRUCT(BlueprintType)
struct MORPHSPLAYER_API FMorphData
{
	GENERATED_BODY()

	/** The name of morph. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FName Morph = NAME_None;

	/** In which way the morph should be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	EPlaybackType PlaybackType = EPlaybackType::Max;

	/** The initial value of a morph to be set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Start (A)", ClampMin = "0", ClampMax = "1"))
	float StartValue = 0.f;

	/** The finish value of a morph to be set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "End (B)", ClampMin = "0", ClampMax = "1"))
	float EndValue = 1.f;
};
