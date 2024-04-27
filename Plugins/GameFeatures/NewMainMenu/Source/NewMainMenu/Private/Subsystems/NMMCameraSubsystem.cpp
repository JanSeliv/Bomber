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
	const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
	return CurrentSpot ? UGameplayUtilsLibrary::GetAttachedActorByClass<ACameraActor>(GetCurrentRailRig()) : nullptr;
}

// Returns attached Rail of this spot that follows the camera to the next spot
ACineCameraRigRail* UNMMCameraSubsystem::GetCurrentRailRig()
{
	// The Rail Rig is attached right to the spot
	const UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	const UNMMSpotComponent* MenuSpot = IsCameraForwardTransition()
		                                    ? SpotsSubsystem.GetCurrentSpot()
		                                    : SpotsSubsystem.GetNextSpot(1, UMyBlueprintFunctionLibrary::GetLevelType());
	return MenuSpot ? UGameplayUtilsLibrary::GetAttachedActorByClass<ACineCameraRigRail>(MenuSpot->GetOwner()) : nullptr;
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
	const float CameraBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();
	constexpr float InstantBlendTime = 0.f;

	switch (MainMenuState)
	{
	case ENMMState::Transition:
		BlendParams.BlendTime = CameraBlendTime;
		break;
	case ENMMState::Idle:
		BlendParams.BlendTime = UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled() ? InstantBlendTime : CameraBlendTime;
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

	if (UNMMUtils::GetMainMenuState() == ENMMState::Transition)
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

	// Change the camera according to the cinematic state
	PossessCamera(NewState);
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

// Is called on starts blending the camera towards current spot on the rail
void UNMMCameraSubsystem::OnBeginTransition()
{
	ACineCameraRigRail* CurrentRailRig = GetCurrentRailRig();
	if (!ensureMsgf(CurrentRailRig, TEXT("ASSERT: [%i] %s:\n'CurrentRailRig' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Start the transition, so the camera will move to this spot
	CurrentRailRig->SetDriveMode(ECineCameraRigRailDriveMode::Manual);
	CurrentRailRig->bUseAbsolutePosition = true;
	CurrentRailRig->AbsolutePositionOnRail = GetCameraStartTransitionValue();
	PossessCamera(ENMMState::Transition);
}

// Is called on finishes blending the camera towards current spot on the rail
void UNMMCameraSubsystem::OnEndTransition()
{
	// Finish the transition once the camera reaches the spot
	UNMMBaseSubsystem::Get().SetNewMainMenuState(ENMMState::Idle);
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

	if (FMath::IsNearlyEqual(Progress, GetCameraLastTransitionValue(), KINDA_SMALL_NUMBER))
	{
		OnEndTransition();
	}
}
