// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMCameraSubsystem.h"
//---
#include "NMMUtils.h"
#include "Components/MyCameraComponent.h"
#include "Components/NMMSpotComponent.h"
#include "Controllers/MyPlayerController.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
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
			return CurrentSpot ? UCinematicUtils::FindSequenceCameraComponent(CurrentSpot->GetMasterPlayer()) : nullptr;
		}
	default: break;
	}

	return nullptr;
}

// Returns attached Rail Camera of this spot that follows the camera to the next spot
ACameraActor* UNMMCameraSubsystem::GetCurrentRailCamera()
{
	// The Rail Camera is attached to the Rail Rig (not to the Spot directly)
	constexpr bool bIncludeDescendants = true;
	const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	return CurrentSpot ? UUtilsLibrary::GetAttachedActorByClass<ACameraActor>(CurrentSpot->GetOwner(), bIncludeDescendants) : nullptr;
}

// Returns attached Rail of this spot that follows the camera to the next spot
ACineCameraRigRail* UNMMCameraSubsystem::GetCurrentRailRig()
{
	// The Rail Rig is attached right to the spot
	constexpr bool bIncludeDescendants = false;
	const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	return CurrentSpot ? UUtilsLibrary::GetAttachedActorByClass<ACineCameraRigRail>(CurrentSpot->GetOwner(), bIncludeDescendants) : nullptr;
}

// Starts blending the camera towards this spot on the rail
void UNMMCameraSubsystem::BeginCameraRailTransition()
{
	ACineCameraRigRail* CurrentRailRig = GetCurrentRailRig();
	if (!ensureMsgf(CurrentRailRig, TEXT("ASSERT: [%i] %s:\n'CurrentRailRig' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Start the transition, so the camera will move to this spot
	CurrentRailRig->SetDriveMode(ECineCameraRigRailDriveMode::Duration);
	CurrentRailRig->AbsolutePositionOnRail = 0.f;
	CurrentRailRig->CurrentPositionOnRail = 0.f;
	PossessCamera(ENMMState::Transition);

	// Finish the transition once the camera reaches the spot
	const USplineComponent* SplineComp = CurrentRailRig->GetRailSplineComponent();
	checkf(SplineComp, TEXT("ERROR: [%i] %s:\n'SplineComp' is null!"), __LINE__, *FString(__FUNCTION__));
	const float TransitionDuration = SplineComp->Duration;
	auto OnTransitionFinished = []
	{
		UNMMBaseSubsystem::Get().SetNewMainMenuState(ENMMState::Idle);
	};
	FTimerHandle OnTransitionFinishedHandle;
	GetWorldRef().GetTimerManager().SetTimer(OnTransitionFinishedHandle, OnTransitionFinished, TransitionDuration, false);
}

// Starts viewing through camera of current cinematic
void UNMMCameraSubsystem::PossessCamera(ENMMState MainMenuState)
{
	AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
	const UCameraComponent* CameraComponent = FindCameraComponent(MainMenuState);
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %s:\n'MyPC' is not valid!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(CameraComponent, TEXT("ASSERT: [%i] %s:\n'CameraComponent' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	FViewTargetTransitionParams BlendParams;
	switch (MainMenuState)
	{
	case ENMMState::Transition:
		BlendParams.BlendTime = 0.25f;
		break;
	case ENMMState::Idle:
		BlendParams.BlendTime = UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled() ? 0.f : 0.25f;
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

	// Move here the camera towards the next spot on the rail
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
		BeginCameraRailTransition();
		break;
	default: break;
	}

	// Change the camera according to the cinematic state
	PossessCamera(NewState);
}
