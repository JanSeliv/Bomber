// Copyright 2021 Yevhenii Selivanov

#include "Components/MyCameraComponent.h"
//---
#include "GeneratedMap.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
//---
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
UMyCameraComponent::UMyCameraComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Set transform defaults
	SetRelativeLocation(FVector(0.F, 0.F, 1000.F));
	SetRelativeRotation(FRotator(-90.0F, 0.0F, -90.0F));
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
	if (const AGeneratedMap* LevelMap = Cast<AGeneratedMap>(GetOwner()))
	{
		const float Multiplier = 1.5f;
		MaxHeightInternal = FCell::CellSize * LevelMap->GetActorScale3D().GetMax() * Multiplier;
	}
}

// Set the location between players
bool UMyCameraComponent::UpdateLocation(float DeltaTime/* = 0.f*/)
{
	FVector NewLocation = FVector::ZeroVector;

	// If true, the camera will be forced moving to the start position
	const AGeneratedMap* LevelMap = Cast<AGeneratedMap>(GetOwner());
	if (!LevelMap
	    || !LevelMap->GetAlivePlayersNum()
	    || bForceStartInternal)
	{
		NewLocation = FMath::Lerp(GetComponentLocation(), StartLocationInternal, DeltaTime);
		SetWorldLocation(NewLocation);

		// return false to disable tick on finishing
		return !NewLocation.Equals(StartLocationInternal, 10.f);
	}

	// Distance finding between players
	float Distance = 0;
	FCells PlayersCells;
	LevelMap->IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));
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
	NewLocation = USingletonLibrary::GetCellArrayAverage(PlayersCells).Location;
	NewLocation.Z = FMath::Max(MinHeight, NewLocation.Z + Distance);
	if (DeltaTime)
	{
		NewLocation = FMath::Lerp(GetComponentLocation(), NewLocation, DeltaTime);
	}
	SetWorldLocation(NewLocation);

	return true;
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

	StartLocationInternal = GetComponentLocation();

	// Listen states to manage the tick
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

//
void UMyCameraComponent::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	bool bShouldTick = false;

	switch (CurrentGameState)
	{
		case ECurrentGameState::GameStarting:
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
			const AMyGameStateBase* GameStateBase = USingletonLibrary::GetMyGameState(this);
			if (PlayerController
			    && GameStateBase)
			{
				PlayerController->SetViewTargetWithBlend(GetOwner(), GameStateBase->GetStartingCountdown());
			}
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
		default: break;
	}

	SetComponentTickEnabled(bShouldTick);
}
