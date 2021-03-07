// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/TimelineComponent.h"
//---
#include "AnimNotifyState_PlayMorph.generated.h"

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
UCLASS(Blueprintable, meta = (DisplayName = "Play Morph"))
class BOMBER_API UAnimNotifyState_PlayMorph final : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Overridden from UAnimNotifyState to provide custom notify name. */
	virtual FString GetNotifyName_Implementation() const override;

	/**
	 * Is called on an animation notify.
	 * @param MeshComp
	 * @param Animation
	 * @param TotalDuration How long the notify is played. Is determined by amount of frames dragged on an animation track.
	 */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	/**
	 * Is called on an every animation tick.
	 * @param MeshComp
	 * @param Animation
	 * @param FrameDeltaTime
	 */
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;

	/**
	 * Is called before destroying.
	 * @param MeshComp
	 * @param Animation
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/** Returns the whole data about chosen morph.
	 * @see UAnimNotify_PlayMorph::MorphData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FMorphData GetMorphData() const { return MorphDataInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains information to set in details panel about morph and in which way play it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Morph Data", ShowOnlyInnerProperties))
	FMorphData MorphDataInternal;

	/** */
	FTimeline TimelineInternal; //[G]

	/**  */
	UPROPERTY(Transient,  BlueprintReadWrite, Category = "C++")
	class UCurveFloat* CurveFloat;

	/** */
	UPROPERTY(Transient,  BlueprintReadWrite, Category = "C++")
	class USkeletalMeshComponent* MeshCompInternal;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitCurveFloatOnce();

	/**
	 *
	 * @param TotalDuration
	 */
	void UpdateCurveLength(float TotalDuration);

	/**
	 *
	 * @param PlaybackPosition
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void HandlePlayMorph(float PlaybackPosition);
};
