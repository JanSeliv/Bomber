// Copyright 2020 Yevhenii Selivanov

#include "MyCameraActor.h"
//---
#include "GeneratedMap.h"
#include "SingletonLibrary.h"
//---
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyCameraActor::AMyCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set defaults to the CameraComponent
	GetCameraComponent()->SetRelativeLocation(FVector(0.F, 0.F, 500.F));
	GetCameraComponent()->SetRelativeRotation(FRotator(-90.0F, 0.0F, -90.0F));
	GetCameraComponent()->SetConstraintAspectRatio(false);	// viewport without black borders
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
	LevelMap->IntersectCellsByTypes(PlayersCells, TO_FLAG(AT::Player));

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

	const AGeneratedMap* const LevelMap = USingletonLibrary::GetLevelMap();
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

	// Set view
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}
}
