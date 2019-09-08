// Copyright 2019 Yevhenii Selivanov.

#include "LevelActors/BoxActor.h"

#include "Bomber.h"
#include "Components/StaticMeshComponent.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "Math/UnrealMathUtility.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values.
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize box mesh component
	BoxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMeshComponent"));
	BoxMeshComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BombMeshFinder(TEXT("/Game/Bomber/Assets/Meshes/BoxMesh"));
	if (BombMeshFinder.Succeeded())
	{
		BoxMeshComponent->SetStaticMesh(BombMeshFinder.Object);
	}
}

// Called when an instance of this class is placed (in editor) or spawned.
void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(MapComponent) == false)  // this component is not valid for owner construction
	{
		return;
	}

	// Construct the actor's map component.
	MapComponent->OnMapComponentConstruction();
}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABoxActor::OnBoxDestroyed);
}

// Event triggered when the actor has been explicitly destroyed. With some chances spawns an item.
void ABoxActor::OnBoxDestroyed(AActor* DestroyedActor)
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (LevelMap == nullptr					// The Level Map is not accessible
		|| IsValid(MapComponent) == false)  // The Map Component is not valid or transient
	{
		return;
	}

	// Spawn item with the chance
	const bool ItemChance = FMath::RandRange(int32(0), int32(100)) < 30;
	if (ItemChance)
	{
		LevelMap->SpawnActorByType(EActorType::Item, MapComponent->GetCell());
	}

	USingletonLibrary::PrintToLog(this, "OnWallDestroyed", (ItemChance ? "Item spawned" : ""));
}
