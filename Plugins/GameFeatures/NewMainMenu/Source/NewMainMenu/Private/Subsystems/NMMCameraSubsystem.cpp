// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMCameraSubsystem.h"
//---
#include "NMMUtils.h"
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/NMMDataAsset.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "CineCameraRigRail.h"
#include "LevelSequencePlayer.h"
#include "TimerManager.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCameraSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMCameraSubsystem& UNMMCameraSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNMMCameraSubsystem* ThisSubsystem = UNMMUtils::GetCameraSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'CameraSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns current camera component depending on the current Menu state
UCameraComponent* UNMMCameraSubsystem::FindCameraComponent(ENMMState MainMenuState)
{
	switch (MainMenuState)
	{
	case ENMMState::None:
		{
			return UMyBlueprintFunctionLibrary::GetLevelCamera();
		}
	case ENMMState::Transition:
		{
			const ACameraActor* CurrentRailCamera = GetCurrentRailCamera();
			return CurrentRailCamera ? CurrentRailCamera->GetCameraComponent() : nullptr;
		}
	case ENMMState::Idle: // Fall through
	case ENMMState::Cinematic:
		{
			const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
			ULevelSequencePlayer* MasterPlayer = CurrentSpot ? CurrentSpot->GetMasterPlayer() : nullptr;
			return MasterPlayer ? UCinematicUtils::FindSequenceCameraComponent(MasterPlayer) : nullptr;
		}
	default: break;
	}

	return nullptr;
}

// Returns attached Rail Camera of this spot that follows the camera to the next spot
ACameraActor* UNMMCameraSubsystem::GetCurrentRailCamera()
{
	// The Rail Camera is attached to the Rail Rig (not to the Spot directly)
	const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	return CurrentSpot ? UGameplayUtilsLibrary::GetAttachedActorByClass<ACameraActor>(GetCurrentRailRig()) : nullptr;
}

// Returns attached Rail of this spot that follows the camera to the next spot
ACineCameraRigRail* UNMMCameraSubsystem::GetCurrentRailRig()
{
	// The Rail Rig is attached right to the spot
	constexpr int32 ForwardDir = 1;
	const UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	const UNMMSpotComponent* MenuSpot = IsCameraForwardTransition()
		                                    ? SpotsSubsystem.GetCurrentSpot()
		                                    : SpotsSubsystem.GetNextSpot(ForwardDir, UMyBlueprintFunctionLibrary::GetLevelType());
	return MenuSpot ? UGameplayUtilsLibrary::GetAttachedActorByClass<ACineCameraRigRail>(MenuSpot->GetOwner()) : nullptr;
}

// Starts viewing through camera of current cinematic
void UNMMCameraSubsystem::PossessCamera(ENMMState MainMenuState)
{
	const UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	if (!SpotsSubsystem.IsActiveMenuSpotReady())
	{
		// Ignore if there is no active spot initialized yet, it will be called once the spot is ready
		return;
	}

	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	const UCameraComponent* CameraComponent = FindCameraComponent(MainMenuState);
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %s:\n'MyPC' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(CameraComponent, TEXT("ASSERT: [%i] %s:\n'CameraComponent' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| MyPC->GetViewTarget() == CameraComponent->GetOwner()) // Already possessed
	{
		return;
	}

	FViewTargetTransitionParams BlendParams;
	const float CameraBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();

	switch (MainMenuState)
	{
	case ENMMState::Transition:
		BlendParams.BlendTime = CameraBlendTime;
		break;
	case ENMMState::Idle:
		if (!UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled()
			&& SpotsSubsystem.GetLastMoveSpotDirection() != 0)
		{
			BlendParams.BlendTime = CameraBlendTime;
		}
		break;
	default: break;
	}

	MyPC->SetViewTarget(CameraComponent->GetOwner(), BlendParams);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

void UNMMCameraSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Listen Main Menu states
	UNMMBaseSubsystem& BaseSubsystem = UNMMBaseSubsystem::Get();
	BaseSubsystem.OnMainMenuStateChanged.AddUniqueDynamic(this, &ThisClass::OnNewMainMenuStateChanged);
	if (BaseSubsystem.GetCurrentMenuState() != ENMMState::None)
	{
		// State is already set, apply it
		OnNewMainMenuStateChanged(BaseSubsystem.GetCurrentMenuState());
	}
}

// Return the stat id to use for this tickable
TStatId UNMMCameraSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ThisClass, STATGROUP_Tickables);
}

// Is called every frame to move the camera
void UNMMCameraSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsBlendingInOutInternal
		&& UNMMUtils::GetMainMenuState() == ENMMState::Transition)
	{
		TickTransition(DeltaTime);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the Main Menu state was changed
void UNMMCameraSubsystem::OnNewMainMenuStateChanged_Implementation(ENMMState NewState)
{
	switch (NewState)
	{
	case ENMMState::Transition:
		// Start blending the camera towards current spot on the rail
		OnBeginTransition();
		break;
	default: break;
	}
}

/*********************************************************************************************
 * Transitioning
 ********************************************************************************************* */

// Returns true if the camera should transit to the next spot, otherwise in backward direction
bool UNMMCameraSubsystem::IsCameraForwardTransition()
{
	return UNMMSpotsSubsystem::Get().GetLastMoveSpotDirection() > 0.f;
}

// Returns begin value, where the camera should start moving on the rail
float UNMMCameraSubsystem::GetCameraStartTransitionValue()
{
	return IsCameraForwardTransition() ? 0.f : 1.f;
}

// Returns end value, where the camera should stop moving on the rail
float UNMMCameraSubsystem::GetCameraLastTransitionValue()
{
	return IsCameraForwardTransition() ? 1.f : 0.f;
}

// Applies the new state of camera rail transition state
void UNMMCameraSubsystem::SetNewCameraRailTransitionState(ENMMCameraRailTransitionState NewCameraRailState)
{
	if (NewCameraRailState == CameraRailTransitionStateInternal)
	{
		return;
	}

	CameraRailTransitionStateInternal = NewCameraRailState;
	OnCameraRailTransitionStateChanged.Broadcast(CameraRailTransitionStateInternal);
}

// Is called on starts blending the camera towards current spot on the rail
void UNMMCameraSubsystem::OnBeginTransition()
{
	ACineCameraRigRail* CurrentRailRig = GetCurrentRailRig();
	if (!ensureMsgf(CurrentRailRig, TEXT("ASSERT: [%i] %s:\n'CurrentRailRig' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Setup the rail readiness 
	CurrentRailRig->SetDriveMode(ECineCameraRigRailDriveMode::Manual);
	CurrentRailRig->bUseAbsolutePosition = true;
	CurrentRailRig->AbsolutePositionOnRail = GetCameraStartTransitionValue();

	// Transition state is started, so we need to blend the gap between initial spot and beginning of the rail
	bIsBlendingInOutInternal = true;
	PossessCamera(ENMMState::Transition);

	// Trigger rail once the camera is blended
	FTimerHandle BlendTimerHandle;
	const float TransitionToIdleBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();
	GetWorldRef().GetTimerManager().SetTimer(BlendTimerHandle, []
	{
		Get().bIsBlendingInOutInternal = false;
	}, TransitionToIdleBlendTime, false);

	SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::BeginTransition);
}

// Is called on finishes blending the camera towards current spot on the rail
void UNMMCameraSubsystem::OnEndTransition()
{
	// Rail is finish, so we need to blend the gap between the end of rail and the camera spot
	bIsBlendingInOutInternal = true;
	PossessCamera(ENMMState::Idle);

	// Finish Transition state once the camera is blended
	FTimerHandle BlendTimerHandle;
	const float TransitionToIdleBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();
	GetWorldRef().GetTimerManager().SetTimer(BlendTimerHandle, []
	{
		Get().bIsBlendingInOutInternal = false;
		UNMMBaseSubsystem::Get().SetNewMainMenuState(ENMMState::Idle);
	}, TransitionToIdleBlendTime, false);

	SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::EndTransition);
}

// Is called in tick to update the camera transition when transitioning
void UNMMCameraSubsystem::TickTransition(float DeltaTime)
{
	ACineCameraRigRail* CurrentRailRig = GetCurrentRailRig();
	const float CameraTransitionTime = UNMMDataAsset::Get().GetCameraTransitionTime();
	if (!CurrentRailRig
		|| !ensureMsgf(CameraTransitionTime > 0.f, TEXT("ASSERT: [%i] %s:\n''CameraTransitionTime' has to be greater than 0!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	float Progress = DeltaTime / CameraTransitionTime;
	Progress = IsCameraForwardTransition() ? CurrentRailRig->AbsolutePositionOnRail + Progress : CurrentRailRig->AbsolutePositionOnRail - Progress;
	Progress = FMath::Clamp(Progress, 0.f, 1.f);

	CurrentRailRig->AbsolutePositionOnRail = Progress;

	// checks if it's halfway of transition
	constexpr float HalfwayPosition = 0.5f;
	if (CameraRailTransitionStateInternal != ENMMCameraRailTransitionState::HalfwayTransition &&
		FMath::IsNearlyEqual(Progress, HalfwayPosition, DeltaTime))
	{
		SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::HalfwayTransition);
	}

	// continue execution to ensure full camera movement 
	if (FMath::IsNearlyEqual(Progress, GetCameraLastTransitionValue(), KINDA_SMALL_NUMBER))
	{
		OnEndTransition();
	}
}
