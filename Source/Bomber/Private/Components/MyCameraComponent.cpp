// Copyright (c) Yevhenii Selivanov

#include "Components/MyCameraComponent.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/GameStateDataAsset.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "Camera/CameraComponent.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#endif

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
	bCameraMeshHiddenInGame = !UEditorUtilsLibrary::IsEditor();
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
	const float MaxLevelScale = AGeneratedMap::Get().GetActorScale3D().GetMax();
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
		const FVector CameraLockedLocation = GetCameraLockedLocation();
		const bool bShouldLerp = !CameraWorldLocation.Equals(CameraLockedLocation, Tolerance);
		if (bShouldLerp)
		{
			NewLocation = FMath::Lerp(CameraWorldLocation, CameraLockedLocation, DeltaTime);
			SetWorldLocation(NewLocation);
		}

		// return false to disable tick on finishing
		return bShouldLerp;
	}

	// Distance finding between players
	NewLocation = GetCameraLocationBetweenPlayers();

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

// Returns the center camera location between all specified cells
FVector UMyCameraComponent::GetCameraLocationBetweenCells(const FCells& Cells) const
{
	float Distance = FCell::GetCellArrayMaxDistance<float>(Cells);
	Distance *= FCell::CellSize;
	Distance = FMath::Min(Distance, MaxHeightInternal);

	FVector NewLocation = FCell::GetCellArrayAverage(Cells).Location;
	NewLocation.Z = FMath::Max(MinHeightInternal, NewLocation.Z + Distance);
	return NewLocation;
}

// Returns the center location between all players and bots
FVector UMyCameraComponent::GetCameraLocationBetweenPlayers() const
{
	const FCells PlayersCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Player));
	return GetCameraLocationBetweenCells(PlayersCells);
}

// Returns the default location between all players and bots
FVector UMyCameraComponent::GetCameraLockedLocation() const
{
	constexpr int32 FirstCellIndex = 0;
	const int32 LastColumnIndex = UCellsUtilsLibrary::GetLastColumnIndexOnLevel();
	const int32 LastRowIndex = UCellsUtilsLibrary::GetLastRowIndexOnLevel();

	const FCells CornerCells = {
		UCellsUtilsLibrary::GetCellOnLevel(FirstCellIndex, FirstCellIndex),
		UCellsUtilsLibrary::GetCellOnLevel(FirstCellIndex, LastColumnIndex),
		UCellsUtilsLibrary::GetCellOnLevel(LastRowIndex, FirstCellIndex),
		UCellsUtilsLibrary::GetCellOnLevel(LastRowIndex, LastColumnIndex)
	};

	return GetCameraLocationBetweenCells(CornerCells);
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
