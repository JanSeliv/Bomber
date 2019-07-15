// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxActor.h"

#include "Bomber.h"
#include "Components/StaticMeshComponent.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "Math/UnrealMathUtility.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize box mesh
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BoxMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BombMeshFinder(TEXT("/Game/Bomber/Assets/Meshes/BoxMesh"));
	if (BombMeshFinder.Succeeded())
	{
		BoxMesh->SetStaticMesh(BombMeshFinder.Object);
	}
}

void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_VALID(MapComponent) == false)
	{
		return;
	}

	// Update this actor
	MapComponent->UpdateSelfOnMap();
}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABoxActor::OnBoxDestroyed);
}

void ABoxActor::OnBoxDestroyed(AActor* DestroyedActor)
{
	// Spawn item with the chance
	const bool ItemChance = FMath::RandRange(int32(0), int32(100)) < 30;
	if (ItemChance)
	{
		USingletonLibrary::GetLevelMap(GetWorld())
			->AddActorOnMap(GetActorTransform(), EActorTypeEnum::Item);
	}

	UE_LOG_STR(this, "OnBoxDestroyed", (ItemChance ? "Item spawned" : ""));
}
