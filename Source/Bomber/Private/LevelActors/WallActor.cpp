// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/WallActor.h"
//---
#include "Bomber.h"
#include "Components/MapComponent.h"

// Default constructor
UWallDataAsset::UWallDataAsset()
{
	ActorTypeInternal = EAT::Wall;
}

// Sets default values
AWallActor::AWallActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
}

// Called when an instance of this class is placed (in editor) or spawned
void AWallActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)			// This actor is transient
		|| !IsValid(MapComponentInternal))	// Is not valid for map construction
	{
		return;
	}

	// Update this actor on the Level Map
	MapComponentInternal->OnConstruction();
}

// Called when the game starts or when spawned
void AWallActor::BeginPlay()
{
	Super::BeginPlay();
}
