// Copyright (c) Yevhenii Selivanov

#include "Subsystems/NMMCameraSubsystem.h"

// NMM
#include "Components/NMMSpotComponent.h"
#include "Data/NMMDataAsset.h"
#include "Data/NmmStateTag.h"
#include "NMMUtils.h"
#include "NmmGameplayTags.h"
#include "Subsystems/NMMBaseSubsystem.h"
#include "Subsystems/NMMInGameSettingsSubsystem.h"
#include "Subsystems/NMMSpotsSubsystem.h"

// Bomber
#include "Components/BmrCameraComponent.h"
#include "Controllers/BmrPlayerController.h"
#include "MyUtilsLibraries/CinematicUtils.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Camera/CameraActor.h"
#include "CineCameraRigRail.h"
#include "Engine/World.h"
#include "LevelSequencePlayer.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NMMCameraSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UNMMCameraSubsystem& UNMMCameraSubsystem::Get(const UObject* OptionalWorldContext)
{
	UNMMCameraSubsystem* ThisSubsystem = UNMMUtils::GetCameraSubsystem(OptionalWorldContext);
	checkf(ThisSubsystem, TEXT("%s: 'CameraSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns current camera component depending on the current Menu state
UCameraComponent* UNMMCameraSubsystem::FindCameraComponent(FNmmStateTag MenuState)
{
	if (!MenuState.IsValid()
	    || MenuState == FNmmStateTag::BasicMenu)
	{
		return UBmrBlueprintFunctionLibrary::GetLevelCamera();
	}

	if (MenuState == FNmmStateTag::Transition)
	{
		const ACameraActor* CurrentRailCamera = GetCurrentRailCamera();
		return CurrentRailCamera ? CurrentRailCamera->GetCameraComponent() : nullptr;
	}

	if ((FNmmStateTag::Idle | FNmmStateTag::Cinematic).HasTag(MenuState))
	{
		const UNMMSpotComponent* CurrentSpot = UNMMSpotsSubsystem::Get().GetCurrentSpot();
		ULevelSequencePlayer* MasterPlayer = CurrentSpot ? CurrentSpot->GetMasterPlayer() : nullptr;
		return MasterPlayer ? UCinematicUtils::FindSequenceCameraComponent(MasterPlayer) : nullptr;
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
	const UNMMSpotComponent* MenuSpot = IsCameraForwardTransition() ? SpotsSubsystem.GetCurrentSpot() : SpotsSubsystem.GetNextSpot(ForwardDir);
	return MenuSpot ? UGameplayUtilsLibrary::GetAttachedActorByClass<ACineCameraRigRail>(MenuSpot->GetOwner()) : nullptr;
}

// Starts viewing through camera of current cinematic
void UNMMCameraSubsystem::PossessCamera(FNmmStateTag MenuState)
{
	const UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	if (!SpotsSubsystem.IsActiveMenuSpotReady()
	    && MenuState != FNmmStateTag::BasicMenu
	    && MenuState.IsValid())
	{
		// Ignore if there is no active spot initialized yet, it will be called once the spot is ready
		// BasicMenu and None target the level camera and do not require a spot
		return;
	}

	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	const UCameraComponent* CameraComponent = FindCameraComponent(MenuState);
	if (!ensureMsgf(MyPC, TEXT("ASSERT: [%i] %s:\n'MyPC' is not valid!"), __LINE__, *FString(__FUNCTION__))
	    || !ensureMsgf(CameraComponent, TEXT("ASSERT: [%i] %s:\n'CameraComponent' is not valid!"), __LINE__, *FString(__FUNCTION__))
	    || MyPC->GetViewTarget() == CameraComponent->GetOwner()) // Already possessed
	{
		return;
	}

	FViewTargetTransitionParams BlendParams;
	const float CameraBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();

	if (MenuState == FNmmStateTag::Transition)
	{
		BlendParams.BlendTime = CameraBlendTime;
	}
	else if (MenuState == FNmmStateTag::Idle
	         && !UNMMInGameSettingsSubsystem::Get().IsInstantCharacterSwitchEnabled()
	         && SpotsSubsystem.GetLastMoveSpotDirection() != 0)
	{
		BlendParams.BlendTime = CameraBlendTime;
	}

	MyPC->SetViewTarget(CameraComponent->GetOwner(), BlendParams);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Subscribes to menu state events
void UNMMCameraSubsystem::OnGameFeatureInitialize_Implementation()
{
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(NmmGameplayTags::Event::MenuStateChanged, this, &ThisClass::OnNewMainMenuStateChanged);

	// Listen for spot readiness to possess camera when async-loaded spot becomes available
	UNMMSpotsSubsystem& SpotsSubsystem = UNMMSpotsSubsystem::Get();
	SpotsSubsystem.OnActiveMenuSpotReady.AddUniqueDynamic(this, &ThisClass::OnActiveMenuSpotReady);
}

// Clears all transient data contained in this subsystem
void UNMMCameraSubsystem::OnGameFeatureDeinitialize_Implementation()
{
	UGlobalMessageSubsystem::StopListeningForAllGlobalMessages(this);

	if (UNMMSpotsSubsystem* SpotsSubsystem = UNMMUtils::GetSpotsSubsystem())
	{
		SpotsSubsystem->OnActiveMenuSpotReady.RemoveAll(this);
	}
}

// Is called every frame to move the camera
void UNMMCameraSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsBlendingInOut
	    && UNMMUtils::GetMainMenuState() == FNmmStateTag::Transition)
	{
		TickTransition(DeltaTime);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when the Main Menu state was changed
void UNMMCameraSubsystem::OnNewMainMenuStateChanged_Implementation(const FGameplayEventData& Payload)
{
	const FNmmStateTag NewState(Payload.InstigatorTags.First());

	if (NewState == FNmmStateTag::BasicMenu
	    || !NewState.IsValid())
	{
		// BasicMenu and None both target the level camera
		// Idle/Cinematic possession flows through OnActiveMenuSpotReady after the spot plays
		PossessCamera(NewState);
	}
	else if (NewState == FNmmStateTag::Transition)
	{
		// Start blending the camera towards current spot on the rail
		OnBeginTransition();
	}
}

// Called when a cinematic spot finished loading, possesses camera if it was deferred
void UNMMCameraSubsystem::OnActiveMenuSpotReady_Implementation(UNMMSpotComponent* /*MainMenuSpotComponent*/)
{
	const FNmmStateTag CurrentState = UNMMBaseSubsystem::Get().GetCurrentMenuState();
	if ((FNmmStateTag::Idle | FNmmStateTag::Cinematic).HasTag(CurrentState))
	{
		PossessCamera(CurrentState);
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
	if (NewCameraRailState == CameraRailTransitionState)
	{
		return;
	}

	CameraRailTransitionState = NewCameraRailState;
	OnCameraRailTransitionStateChanged.Broadcast(CameraRailTransitionState);
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
	bIsBlendingInOut = true;
	PossessCamera(FNmmStateTag::Transition);

	// Trigger rail once the camera is blended
	FTimerHandle BlendTimerHandle;
	const float TransitionToIdleBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();
	GetWorldRef().GetTimerManager().SetTimer(BlendTimerHandle, []
	{
		Get().bIsBlendingInOut = false;
	}, TransitionToIdleBlendTime, false);

	SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::BeginTransition);

	SetTickableTickType(ETickableTickType::Conditional);
}

// Is called on finishes blending the camera towards current spot on the rail
void UNMMCameraSubsystem::OnEndTransition()
{
	// Rail is finish, so we need to blend the gap between the end of rail and the camera spot
	bIsBlendingInOut = true;
	PossessCamera(FNmmStateTag::Idle);

	// Finish Transition state once the camera is blended
	FTimerHandle BlendTimerHandle;
	const float TransitionToIdleBlendTime = UNMMDataAsset::Get().GetCameraBlendTime();
	GetWorldRef().GetTimerManager().SetTimer(BlendTimerHandle, []
	{
		Get().bIsBlendingInOut = false;
		UNMMBaseSubsystem::Get().SetNewMainMenuState(FNmmStateTag::Idle);
	}, TransitionToIdleBlendTime, false);

	SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::EndTransition);

	SetTickableTickType(ETickableTickType::Never);
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
	if (CameraRailTransitionState != ENMMCameraRailTransitionState::HalfwayTransition
	    && FMath::IsNearlyEqual(Progress, HalfwayPosition, DeltaTime))
	{
		SetNewCameraRailTransitionState(ENMMCameraRailTransitionState::HalfwayTransition);
	}

	// continue execution to ensure full camera movement
	if (FMath::IsNearlyEqual(Progress, GetCameraLastTransitionValue(), KINDA_SMALL_NUMBER))
	{
		OnEndTransition();
	}
}
