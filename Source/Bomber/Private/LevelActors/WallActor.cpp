// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/WallActor.h"
//---
#include "Components/MapComponent.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(WallActor)

// Sets default values
AWallActor::AWallActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	NetUpdateFrequency = 10.f;
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
}

// Initialize a wall actor, could be called multiple times
void AWallActor::ConstructWallActor()
{
	checkf(MapComponentInternal, TEXT("%s: 'MapComponentInternal' is null"), *FString(__FUNCTION__));
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionWallActor);
	MapComponentInternal->ConstructOwnerActor();
}

// Called when an instance of this class is placed (in editor) or spawned
void AWallActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructWallActor();
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void AWallActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (!bNewHidden)
	{
		// Is added on Generated Map
		ConstructWallActor();
	}
}
