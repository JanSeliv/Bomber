// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
//---
#include "AnimNotify_PlayMorph.generated.h"

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

//ENUM_RANGE_BY_FIRST_AND_LAST($ENUM$, $ENUM$::First, $ENUM$::Last);
/**
 * Data about morph target (shape key)
 */
USTRUCT(BlueprintType)
struct FMorphData
{
	GENERATED_BODY()

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FName Morph;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	EPlaybackType PlaybackType = EPlaybackType::Max;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "Start (A)", ClampMin = "0", ClampMax = "1"))
	float StartValue = 0.f;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties, DisplayName = "End (B)", ClampMin = "0", ClampMax = "1"))
	float EndValue = 1.f;
};

/**
 *
 */
UCLASS()
class BOMBER_API UAnimNotify_PlayMorph final : public UAnimNotify
{
	GENERATED_BODY()

public:
	/**
	* @see UAnimNotify_PlayMorph::MorphData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FMorphData GetMorphData() const { return MorphDataInternal; }

	/** Is called on an animation notify. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Morph Data", ShowOnlyInnerProperties))
	FMorphData MorphDataInternal;
};
