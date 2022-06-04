// Copyright (c) Yevhenii Selivanov

#include "Components/MyCameraComponent.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "Controllers/MyPlayerController.h"
//---
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
UMyCameraComponent::UMyCameraComponent()
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
#if WITH_EDITOR // [Editor]
	bCameraMeshHiddenInGame = !USingletonLibrary::IsEditor();
#endif

	// Disable Eye Adaptation
	PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	PostProcessSettings.AutoExposureMinBrightness = 1;
	PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	PostProcessSettings.AutoExposureMaxBrightness = 1;
}

// Set the maximum possible height
void UMyCameraComponent::UpdateMaxHeight()
{
	static constexpr float Multiplier = 1.5f;
	const float MaxLevelScale = AGeneratedMap::Get().GetCachedTransform().GetScale3D().GetMax();
	MaxHeightInternal = FCell::CellSize * MaxLevelScale * Multiplier;
}

// Set the location between players
bool UMyCameraComponent::UpdateLocation(float DeltaTime/* = 0.f*/)
{
	FVector NewLocation = FVector::ZeroVector;

	// If true, the camera will be forced moving to the start position
	if (bIsCameraLockedOnCenterInternal
	    || !USingletonLibrary::GetAlivePlayersNum()
	    || bForceStartInternal)
	{
		static constexpr float Tolerance = 10.f;
		const FVector CameraWorldLocation = GetComponentLocation();
		const bool bShouldLerp = !CameraWorldLocation.Equals(StartLocationInternal, Tolerance);
		if (bShouldLerp)
		{
			NewLocation = FMath::Lerp(CameraWorldLocation, StartLocationInternal, DeltaTime);
			SetWorldLocation(NewLocation);
		}

		// return false to disable tick on finishing
		return bShouldLerp;
	}

	// Distance finding between players
	NewLocation = GetLocationBetweenPlayers();

	if (DeltaTime)
	{
		NewLocation = FMath::Lerp(GetComponentLocation(), NewLocation, DeltaTime);
	}

	SetWorldLocation(NewLocation);

	return true;
}

// Calls to set following camera by player locations
void UMyCameraComponent::SetCameraLockedOnCenter(bool bInCameraLockedOnCenter)
{
	bIsCameraLockedOnCenterInternal = bInCameraLockedOnCenter;

	// Enable camera if should be unlocked
	if (!bInCameraLockedOnCenter
	    && !IsComponentTickEnabled()
	    && AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::InGame)
	{
		SetComponentTickEnabled(true);
	}
}

// Returns the center location between all players and bots
FVector UMyCameraComponent::GetLocationBetweenPlayers() const
{
	float Distance = 0.f;
	FCells PlayersCells;
	AGeneratedMap::Get().IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));
	for (const FCell& C1 : PlayersCells)
	{
		for (const FCell& C2 : PlayersCells)
		{
			const float LengthIt = USingletonLibrary::CalculateCellsLength(C1, C2);
			if (LengthIt > Distance)
			{
				Distance = LengthIt;
			}
		}
	}
	Distance *= FCell::CellSize;
	if (Distance > MaxHeightInternal)
	{
		Distance = MaxHeightInternal;
	}

	// Set the new location
	FVector NewLocation = FVector::ZeroVector;
	NewLocation = USingletonLibrary::GetCellArrayAverage(PlayersCells).Location;
	NewLocation.Z = FMath::Max(MinHeightInternal, NewLocation.Z + Distance);
	return NewLocation;
}

// Called every frame
void UMyCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdateLocation(DeltaTime))
	{
		SetComponentTickEnabled(false);
	}
}

// Called when the game starts or when spawned
void UMyCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Listen game states to manage the tick
void UMyCameraComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	bool bShouldTick = false;

	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			if (StartLocationInternal.IsZero())
			{
				StartLocationInternal = GetLocationBetweenPlayers();
			}
			PossessCamera();
			bShouldTick = true;
			break;
		}
		case ECurrentGameState::EndGame:
		{
			bForceStartInternal = true;
			bShouldTick = true;
			break;
		}
		case ECurrentGameState::InGame:
		{
			bForceStartInternal = false;
			bShouldTick = true;
			break;
		}
		default:
			break;
	}

	SetComponentTickEnabled(bShouldTick);
}

// Starts viewing through this camera
void UMyCameraComponent::PossessCamera()
{
	AActor* Owner = GetOwner();
	AMyPlayerController* MyPC = USingletonLibrary::GetLocalPlayerController();
	if (!ensureMsgf(Owner, TEXT("ASSERT: 'Owner' is not valid"))
	    || !ensureMsgf(MyPC, TEXT("ASSERT: 'MyPC' is not valid")))
	{
		return;
	}

	const float BlendTime = UGameStateDataAsset::Get().GetStartingCountdown();
	MyPC->SetViewTargetWithBlend(Owner, BlendTime);
}
