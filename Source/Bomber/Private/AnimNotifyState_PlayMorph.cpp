// Copyright 2021 Yevhenii Selivanov.
#pragma optimize("", off)
#include "AnimNotifyState_PlayMorph.h"
//
#include "Curves/CurveFloat.h"

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

	UpdateCurveLength(TotalDuration);

	TimelineInternal.PlayFromStart();
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

//
void UAnimNotifyState_PlayMorph::InitCurveFloatOnce()
{
	// return if is already exist
	if (CurveFloat)
	{
		return;
	}

	CurveFloat = NewObject<UCurveFloat>(this, NAME_None, RF_Public | RF_Transactional);

	FOnTimelineFloat OnTimelineFloat;
	OnTimelineFloat.BindDynamic(this, &ThisClass::HandlePlayMorph);

	TimelineInternal.AddInterpFloat(CurveFloat, OnTimelineFloat);
}

//
void UAnimNotifyState_PlayMorph::UpdateCurveLength(float TotalDuration)
{
	if (!ensureMsgf(CurveFloat, TEXT("ASSERT: 'CurveFloat' is not valid")))
	{
		return;
	}

	FRichCurve& Curve = CurveFloat->FloatCurve;
	TArray<FRichCurveKey>& Keys = Curve.Keys;
	if (Keys.Num())
	{
		// Update [1]
		Keys.RemoveAt(Keys.Num() - 1);
	}
	else
	{
		// Set [0]
		Curve.AddKey(0.f, 0.f);
	}
	Curve.AddKey(TotalDuration, 1.f);
}

//
void UAnimNotifyState_PlayMorph::HandlePlayMorph(float PlaybackPosition)
{
	if (!ensureMsgf(MeshCompInternal, TEXT("ASSERT: 'SkeletalMeshComponent' is not valid")))
	{
		return;
	}

	const bool bIsReversing = TimelineInternal.IsReversing();
	static const FVector2D ForwardCurveValue = FVector2D(0.f, 1.f);
	static const FVector2D ReverseCurveValue = FVector2D(1.f, 0.f);
	const FVector2D StartEndCurveValue = bIsReversing ? ReverseCurveValue : ForwardCurveValue;
	const FVector2D StartEndMorphValue = FVector2D(MorphDataInternal.StartValue, MorphDataInternal.EndValue);
	const float NewMorphValue = FMath::GetMappedRangeValueClamped(StartEndCurveValue, StartEndMorphValue, PlaybackPosition);
	MeshCompInternal->SetMorphTarget(MorphDataInternal.Morph, NewMorphValue);
	UE_LOG(LogInit, Log, TEXT("--- StartEndMorphValue: %s, PlaybackPosition: %f -> NewMorphValue: %f"), *StartEndMorphValue.ToString(), PlaybackPosition, NewMorphValue);

}
#pragma optimize("", on)
