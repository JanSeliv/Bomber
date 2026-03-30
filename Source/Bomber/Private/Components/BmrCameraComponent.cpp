// Copyright (c) Yevhenii Selivanov

#include "Components/BmrCameraComponent.h"

// Bomber
#include "Bomber.h"
#include "Controllers/BmrPlayerController.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "Engine/BmrGameViewportClient.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Structures/BmrCell.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrCameraComponent)

// If set, returns additional FOV modifier scaled by level size and current screen aspect ratio
void FCameraDistanceParams::CalculateFitViewAdditiveAngle(float& InOutFOV) const
{
	if (!FitViewAdditiveAngle)
	{
		return;
	}

	// Calculate multiplier to fit with aspect ratios of any screen
	constexpr float VerticalModifier = -1.f;
	constexpr float HorizontalModifier = 1.f;
	const bool bIsVerticalScreen = UUtilsLibrary::GetViewportAspectRatioAxisConstraint() == AspectRatio_MaintainXFOV;
	const float AspectRatioMultiplier = bIsVerticalScreen ? VerticalModifier : HorizontalModifier;

	InOutFOV = InOutFOV - FitViewAdditiveAngle * AspectRatioMultiplier;
}

// If set, truncates specified distance to allowed minimal one
void FCameraDistanceParams::LimitToMinDistance(float& InOutCameraDistance) const
{
	if (MinDistance)
	{
		InOutCameraDistance = FMath::Max(InOutCameraDistance, MinDistance);
	}
}

// Calculates the distance how far away the camera should be placed to fit the given view for specified FOV
float FCameraDistanceParams::CalculateDistanceToFitViewToFOV(const FVector2D& ViewSizeUU, float CameraFOV)
{
	const float NewFOV = FMath::Tan(FMath::DegreesToRadians(CameraFOV / 2.f));

	// Find horizontal and vertical distance for levels and screens with any aspect ratios
	// So the camera will be able to align vertical grid to the wide screen as well as horizontal grid to the vertical screen
	const bool bIsWideScreen = UUtilsLibrary::GetViewportAspectRatioAxisConstraint() == AspectRatio_MaintainYFOV;
	const float HorizontalDistance = bIsWideScreen ? ViewSizeUU.Y / NewFOV : (ViewSizeUU.Y / 2.f) * NewFOV; // view is wider than higher on wide or vertical screen
	const float VerticalDistance = bIsWideScreen ? ViewSizeUU.X / (2.f * NewFOV) : ViewSizeUU.X / NewFOV; // view is longer than wider on wide or vertical screen

	return FMath::Max(HorizontalDistance, VerticalDistance);
}

// Sets default values
UBmrCameraComponent::UBmrCameraComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Set transform defaults
	static const FVector DefaultRelativeLocation(0.F, 0.F, 1000.F);
	SetRelativeLocation_Direct(DefaultRelativeLocation);
	static const FRotator DefaultRelativeRotation(-90.0F, 0.0F, -90.0F);
	SetRelativeRotation_Direct(DefaultRelativeRotation);
	SetUsingAbsoluteScale(true);

	// Camera defaults
	SetConstraintAspectRatio(false); // viewport without black borders
#if WITH_EDITOR //[Editor]
	bCameraMeshHiddenInGame = !FEditorUtilsLibrary::IsEditor();
#endif

	// Disable Eye Adaptation
	PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	PostProcessSettings.AutoExposureMinBrightness = 1;
	PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	PostProcessSettings.AutoExposureMaxBrightness = 1;
}

/** Returns current FOV of camera manager that is more reliable than own FOV. */
float UBmrCameraComponent::GetCameraManagerFOV() const
{
	const ABmrPlayerController* PC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	const APlayerCameraManager* PlayerCameraManager = PC ? PC->PlayerCameraManager : nullptr;
	return PlayerCameraManager ? PlayerCameraManager->GetFOVAngle() : FieldOfView;
}

// Set the location between players
bool UBmrCameraComponent::UpdateLocation(float DeltaTime /* = 0.f*/)
{
	auto MoveCamera = [this, DeltaTime](FVector NewLocation)
	{
		if (DeltaTime)
		{
			NewLocation = FMath::Lerp(GetComponentLocation(), NewLocation, DeltaTime);
		}
		SetWorldLocation(NewLocation);
	};

	// If true, the camera will be forced moving to the start position
	if (bIsCameraLockedOnCenter
	    || !UBmrBlueprintFunctionLibrary::GetAlivePlayersNum(EBmrPlayerType::Any)
	    || bForceStart)
	{
		static constexpr float Tolerance = 10.f;
		const FVector CameraWorldLocation = GetComponentLocation();
		const FVector CameraLockedLocation = GetCameraLockedLocation();
		const bool bShouldLerp = !CameraWorldLocation.Equals(CameraLockedLocation, Tolerance);
		if (bShouldLerp)
		{
			MoveCamera(CameraLockedLocation);
		}

		// return false to disable tick on finishing
		return bShouldLerp;
	}

	// Distance finding between players
	MoveCamera(GetCameraLocationBetweenPlayers());

	return true;
}

// Calls to set following camera by player locations
void UBmrCameraComponent::SetCameraLockedOnCenter(bool bInCameraLockedOnCenter)
{
	bIsCameraLockedOnCenter = bInCameraLockedOnCenter;

	// Enable camera if should be unlocked
	if (!bInCameraLockedOnCenter
	    && !IsComponentTickEnabled()
	    && ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::InGame))
	{
		SetComponentTickEnabled(true);
	}
}

// Allows to tweak distance calculation from camera to the level during the game
void UBmrCameraComponent::SetCameraDistanceParams(const FCameraDistanceParams& InCameraDistanceParams)
{
	DistanceParams = InCameraDistanceParams;

	// Update camera location to apply new distance params
	UpdateLocation();
}

// Calculates how faw away the camera should be placed from specified cells
float UBmrCameraComponent::GetCameraDistanceToCells(const FBmrCells& Cells) const
{
	float CurrentFOV = GetCameraManagerFOV();

	// If is set in params, additional FOV modifier will be applied
	DistanceParams.CalculateFitViewAdditiveAngle(/*InOut*/ CurrentFOV);

	// Instead of changing real FOV, we can just change the distance to the camera to avoid the fisheye effect.
	// Calculate how far away the camera should be placed to fit the given view by specified FOV
	const FVector2D ViewSizeUU = FBmrCell::GetCellArraySize(Cells) * FBmrCell::CellSize;
	float CameraDistance = FCameraDistanceParams::CalculateDistanceToFitViewToFOV(ViewSizeUU, CurrentFOV);

	// If is set in params, cut camera distance by min value
	DistanceParams.LimitToMinDistance(/*InOut*/ CameraDistance);

	return CameraDistance;
}

// Returns the center location between all players and bots
FVector UBmrCameraComponent::GetCameraLocationBetweenPlayers() const
{
	const FBmrCells PlayersCells = UBmrCellUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Player));
	FVector NewLocation = FBmrCell::GetCellArrayCenter(PlayersCells).Location;
	NewLocation.Z = GetCameraLockedLocation().Z; // Z = CameraLock.Z: keep Z axis unchanged to avoid zooming in/out
	return NewLocation;
}

// Returns the default location between all players and bots
FVector UBmrCameraComponent::GetCameraLockedLocation() const
{
	const FBmrCells CornerCells = UBmrCellUtilsLibrary::GetCornerCellsOnLevel();
	FVector NewLocation = FBmrCell::GetCellArrayCenter(CornerCells).Location;
	NewLocation.Z += GetCameraDistanceToCells(CornerCells); // Z = Corners.Z + FitDistance: find the distance to fit the view
	return NewLocation;
}

// Called every frame
void UBmrCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdateLocation(DeltaTime))
	{
		SetComponentTickEnabled(false);
	}
}

// Activates the SceneComponent, should be overridden by native child classes
void UBmrCameraComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	SetComponentTickEnabled(false);
}

// Called when the game starts or when spawned
void UBmrCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen states to manage the tick
	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	// Listen to recalculate camera location
	if (UBmrGameViewportClient* GameViewportClient = UBmrBlueprintFunctionLibrary::GetGameViewportClient())
	{
		GameViewportClient->OnAspectRatioChanged.AddUniqueDynamic(this, &ThisClass::OnAspectRatioChanged);
	}
}

// Listen game states to manage the tick
void UBmrCameraComponent::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	bool bShouldTick = false;

	if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::GameStarting))
	{
		if (bAutoPossessCamera)
		{
			PossessCamera();
		}
		bShouldTick = true;
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::EndGame))
	{
		bForceStart = true;
		bShouldTick = true;
	}
	else if (Payload.InstigatorTags.HasTag(FBmrGameStateTag::InGame))
	{
		bForceStart = false;
		bShouldTick = true;
	}

	SetComponentTickEnabled(bShouldTick);
}

// Listen to recalculate camera location when screen aspect ratio was changed
void UBmrCameraComponent::OnAspectRatioChanged_Implementation(float NewAspectRatio, EAspectRatioAxisConstraint NewAxisConstraint)
{
	UpdateLocation();
}

// Starts viewing through this camera
void UBmrCameraComponent::PossessCamera(bool bBlendCamera /* = true*/)
{
	ABmrPlayerController* MyPC = UBmrBlueprintFunctionLibrary::GetLocalPlayerController();
	if (!MyPC)
	{
		// Is not a local player, skip it
		return;
	}

	const float BlendTime = bBlendCamera ? UBmrGameStateDataAsset::Get().GetStartingCountdown() : 0.f;
	MyPC->SetViewTargetWithBlend(GetOwner(), BlendTime);
}

// Disable to prevent automatic possessing on Game Starting state, could be disabled by external systems like to show cinematic etc
void UBmrCameraComponent::SetAutoPossessCameraEnabled(bool bInAutoPossessCamera)
{
	bAutoPossessCamera = bInAutoPossessCamera;
}
