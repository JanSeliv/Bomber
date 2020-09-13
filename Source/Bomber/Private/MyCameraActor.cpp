// Copyright 2020 Yevhenii Selivanov

#include "MyCameraActor.h"
//---
#include "GeneratedMap.h"
#include "SingletonLibrary.h"
#include "GameFramework/MyGameStateBase.h"
//---
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyCameraActor::AMyCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set defaults to the CameraComp
	UCameraComponent* CameraComp = GetCameraComponent();
	checkf(CameraComp, TEXT("ERROR: Camera Component was not initialized"));
	CameraComp->SetRelativeLocation(FVector(0.F, 0.F, 500.F));
	CameraComp->SetRelativeRotation(FRotator(-90.0F, 0.0F, -90.0F));
	CameraComp->SetConstraintAspectRatio(false);	// viewport without black borders

	// Disable Eye Adaptation
	CameraComp->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
	CameraComp->PostProcessSettings.AutoExposureMinBrightness = 1;
	CameraComp->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	CameraComp->PostProcessSettings.AutoExposureMaxBrightness = 1;
}

// Called every frame
void AMyCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap();
	if (IsValid(LevelMap) == false || LevelMap->GetCharactersNum() == 0)
	{
		SetActorTickEnabled(false);
		return;
	}

	// Find all players locations
	FCells PlayersCells;
	LevelMap->IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));

	// Distance finding between players
	float Distance = 0;
	if (PlayersCells.Num() > 1)
	{
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
		if (Distance > MaxHeight)
		{
			Distance = MaxHeight;
		}
	}

	// Set the new location
	FVector NewLocation = USingletonLibrary::GetCellArrayAverage(PlayersCells).Location;
	NewLocation.Z += Distance;
	NewLocation = FMath::Lerp(GetActorLocation(), NewLocation, DeltaTime);
	SetActorLocation(NewLocation);
}

// Called when the game starts or when spawned
void AMyCameraActor::BeginPlay()
{
	Super::BeginPlay();

	const AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IsValid(LevelMap) == false)
	{
		return;
	}

	// Set the max distance
	MaxHeight = FCell::CellSize * LevelMap->GetActorScale3D().GetMax();

	// Set the start location and rotation
	FVector NewLocation = LevelMap->GetActorLocation();
	NewLocation.Z += MaxHeight;
	SetActorLocation(NewLocation);
	SetActorRotation(LevelMap->GetActorRotation());

	// Listen states to manage the tick
	if(AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

//
void AMyCameraActor::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	SetActorTickEnabled(CurrentGameState == ECurrentGameState::InGame);

	if (CurrentGameState == ECurrentGameState::GameStarting)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerController->SetViewTargetWithBlend(this, BlendTimeInternal);
		}
	}
}
