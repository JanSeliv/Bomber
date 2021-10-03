// Copyright 2021 Yevhenii Selivanov.

#include "AnimNotifyState_PlayMorph.h"
//
#include "Components/MySkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Globals/SingletonLibrary.h"

// Overridden from UAnimNotifyState to provide custom notify name
FString UAnimNotifyState_PlayMorph::GetNotifyName_Implementation() const
{
	FString NotifyName = Super::GetNotifyName_Implementation();

	const FName MorphName = MorphDataInternal.Morph;
	if (!MorphName.IsNone())
	{
		NotifyName += "_" + MorphName.ToString();
	}

	return NotifyName;
}

// Is called on an animation notify
void UAnimNotifyState_PlayMorph::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	MeshCompInternal = MeshComp;

	InitCurveFloatOnce();

	ApplyCurveLengthOnce(TotalDuration);

	StartTimeline();
}

// Is called on an every animation tick
void UAnimNotifyState_PlayMorph::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (TimelineInternal.IsPlaying())
	{
		TimelineInternal.TickTimeline(FrameDeltaTime);
	}
}

// Is called before destroying
void UAnimNotifyState_PlayMorph::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}

// Create a new curve, is created once
void UAnimNotifyState_PlayMorph::InitCurveFloatOnce()
{
	// return if is already exist
	if (CurveFloat)
	{
		return;
	}

	CurveFloat = NewObject<UCurveFloat>(this, NAME_None, RF_Public | RF_Transactional);

	FOnTimelineFloat OnTimelineFloat;
	OnTimelineFloat.BindDynamic(this, &ThisClass::OnTimelineTick);
	TimelineInternal.AddInterpFloat(CurveFloat, OnTimelineFloat);

	FOnTimelineEvent OnTimelineFinished;
	OnTimelineFinished.BindDynamic(this, &ThisClass::OnTimelineFinished);
	TimelineInternal.SetTimelineFinishedFunc(OnTimelineFinished);
}

// Set curve values
void UAnimNotifyState_PlayMorph::ApplyCurveLengthOnce(float TotalDuration)
{
	if (!ensureMsgf(CurveFloat, TEXT("ASSERT: 'CurveFloat' is not valid")))
	{
		return;
	}

	FRichCurve& Curve = CurveFloat->FloatCurve;
	TArray<FRichCurveKey>& Keys = Curve.Keys;
	const int32 KeysNum = Keys.Num();
	if (KeysNum) // Is updated before
	{
#if WITH_EDITOR //[IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			// Remove [1] in editor
			Keys.RemoveAt(KeysNum - 1);
		}
		else // Is in game
#endif // WITH_EDITOR
		{
			// Do not change anything during the game
			return;
		}
	}
	else // Is init for first time
	{
		// Set [0]
		Curve.AddKey(0.f, 0.f);
	}

	// Set [1]
	Curve.AddKey(TotalDuration, 1.f);
}

// Is executed on every frame during playing the timeline
void UAnimNotifyState_PlayMorph::OnTimelineTick(float PlaybackPosition)
{
	// If need to play from A to B to A
	if (MorphDataInternal.PlaybackType == EPlaybackType::Max)
	{
		// 2x faster
		PlaybackPosition *= 2;

		static constexpr float MidCurveValue = 1.f;
		if (PlaybackPosition >= MidCurveValue)
		{
			// Reverse playing
			static constexpr float EndCurveValue = 2.f;
			PlaybackPosition = EndCurveValue - PlaybackPosition;
		}
	}

	static const FVector2D StartEndCurveValue = FVector2D(0.f, 1.f);
	const FVector2D StartEndMorphValue = FVector2D(MorphDataInternal.StartValue, MorphDataInternal.EndValue);
	const float NewMorphValue = FMath::GetMappedRangeValueClamped(StartEndCurveValue, StartEndMorphValue, PlaybackPosition);
	SetMorphValue(NewMorphValue);
}

void UAnimNotifyState_PlayMorph::OnTimelineFinished()
{
	// Is called on finished playing
}

// Will play the timeline with current playback type
void UAnimNotifyState_PlayMorph::StartTimeline()
{
	if (MorphDataInternal.Morph.IsNone())
	{
		return;
	}

	switch (MorphDataInternal.PlaybackType)
	{
		case EPlaybackType::None: // Set a specified start value without playing
			SetMorphValue(MorphDataInternal.StartValue);
			break;

		case EPlaybackType::FromStart: // Play from start to end
		case EPlaybackType::Max:       // Play from start to end and back to start
			TimelineInternal.PlayFromStart();
			break;

		case EPlaybackType::FromEnd: // Play from end to start
			TimelineInternal.ReverseFromEnd();
			break;

		default:
			ensure(!"EPlaybackType default case");
			break;
	}
}

// Set a specified value for chosen morph
void UAnimNotifyState_PlayMorph::SetMorphValue(float Value)
{
	if (!ensureMsgf(MeshCompInternal, TEXT("ASSERT: 'SkeletalMeshComponent' is not valid")))
	{
		return;
	}

	const float ClampedValue = FMath::Clamp(Value, MorphDataInternal.StartValue, MorphDataInternal.EndValue);
	MeshCompInternal->SetMorphTarget(MorphDataInternal.Morph, ClampedValue);
}
