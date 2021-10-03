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
 * Data about morph target (shape key) to be played.
 */
USTRUCT(BlueprintType)
struct FMorphData
{
	GENERATED_BODY()

	/** The name of morph. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (ShowOnlyInnerProperties))
	FName Morph;

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

/**
 * Play the chosen morph for picked frames.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Play Morph"))
class UAnimNotifyState_PlayMorph final : public UAnimNotifyState
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
	 * @param MeshComp Current used mesh component.
	 * @param Animation Current played animation.
	 * @param TotalDuration How long the notify is played. Is determined by amount of frames dragged on an animation track.
	 */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	/**
	 * Is called on an every animation tick.
   	 * @param MeshComp Current used mesh component.
	 * @param Animation Current played animation.
	 * @param FrameDeltaTime Tick time.
	 */
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;

	/**
	 * Is called before destroying.
	 * @param MeshComp Current used mesh component.
	 * @param Animation Current played animation.
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/** Returns the whole data about chosen morph.
	 * @see UAnimNotify_PlayMorph::MorphData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FMorphData& GetMorphData() const { return MorphDataInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains information to set in details panel about morph and in which way play it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Morph Data", ShowOnlyInnerProperties))
	FMorphData MorphDataInternal;

	/** Timeline to be played. */
	FTimeline TimelineInternal; //[G]

	/**
	 * Is created once. Can not be set in the editor due to dynamic duration
	 * that depends on length on dragged notify.
	 * Time: [0, TotalDuration].
	 * Values: [0, 1].
	 * @see UAnimNotifyState_PlayMorph::InitCurveFloatOnce()
	 */
	UPROPERTY(Transient, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class UCurveFloat> CurveFloat = nullptr;

	/** The current skeletal mesh. */
	UPROPERTY(Transient, BlueprintReadWrite, Category = "C++")
	TObjectPtr<class USkeletalMeshComponent> MeshCompInternal = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Create a new curve, is created once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitCurveFloatOnce();

	/**
	 * Set curve values.
	 * Is set once during the game, but is overriden in the editor during changes on track.
	 * @param TotalDuration The length of notify.
	 */
	void ApplyCurveLengthOnce(float TotalDuration);

	/**
	 * Is executed on every frame during playing the timeline.
	 * @param PlaybackPosition Current timeline position.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnTimelineTick(float PlaybackPosition);

	/** Is called on finished playing. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnTimelineFinished();

	/**  Will play the timeline with current playback type. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void StartTimeline();

	/**
	 * Set a specified value for chosen morph.
	 * @param Value New morph value between [MorphDataInternal.StartValue, MorphDataInternal.EndValue].
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetMorphValue(float Value);
};
