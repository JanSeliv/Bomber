// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
//---
#include "MorphsPlayerTypes.h"
#include "Components/TimelineComponent.h"
//---
#include "AnimNotifyState_PlayMorph.generated.h"

/**
 * Play the chosen morph for picked frames.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Play Morph"))
class MORPHSPLAYER_API UAnimNotifyState_PlayMorph : public UAnimNotifyState
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
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	/**
	 * Is called on an every animation tick.
   	 * @param MeshComp Current used mesh component.
	 * @param Animation Current played animation.
	 * @param FrameDeltaTime Tick time.
	 */
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	/**
	 * Is called before destroying.
	 * @param MeshComp Current used mesh component.
	 * @param Animation Current played animation.
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Returns the whole data about chosen morph.
	 * @see UAnimNotify_PlayMorph::MorphData */
	UFUNCTION(BlueprintPure, Category = "Morphs Player")
	const FORCEINLINE FMorphData& GetMorphData() const { return MorphDataInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Contains information to set in details panel about morph and in which way play it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morphs Player", meta = (BlueprintProtected, DisplayName = "Morph Data", ShowOnlyInnerProperties))
	FMorphData MorphDataInternal;

	/** Timeline to be played. */
	FTimeline TimelineInternal;

	/**
	 * Is created once. Can not be set in the editor due to dynamic duration
	 * that depends on length on dragged notify.
	 * Time: [0, TotalDuration].
	 * Values: [0, 1].
	 * @see UAnimNotifyState_PlayMorph::InitCurveFloatOnce()
	 */
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Morphs Player")
	TObjectPtr<class UCurveFloat> CurveFloat = nullptr;

	/** The current skeletal mesh. */
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Morphs Player")
	TObjectPtr<class USkeletalMeshComponent> MeshCompInternal = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Create a new curve, is created once. */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void InitCurveFloatOnce();

	/**
	 * Set curve values.
	 * Is set once during the game, but is overriden in the editor during changes on track.
	 * @param TotalDuration The length of notify.
	 */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void ApplyCurveLengthOnce(float TotalDuration);

	/**
	 * Is executed on every frame during playing the timeline.
	 * @param PlaybackPosition Current timeline position.
	 */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void OnTimelineTick(float PlaybackPosition);

	/** Is called on finished playing. */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void OnTimelineFinished();

	/**  Will play the timeline with current playback type. */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void StartTimeline();

	/**
	 * Set a specified value for chosen morph.
	 * @param Value New morph value between [MorphDataInternal.StartValue, MorphDataInternal.EndValue].
	 */
	UFUNCTION(BlueprintCallable, Category = "Morphs Player", meta = (BlueprintProtected))
	virtual void SetMorphValue(float Value);
};
