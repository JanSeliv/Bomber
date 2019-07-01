// Fill out your copyright notice in the Description page of Project Settings.

#include "WallActor.h"

#include "Bomber.h"
#include "MapComponent.h"
// Sets default values
AWallActor::AWallActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize map component
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));
}

void AWallActor::OnConstruction(const FTransform& Transform)
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
void AWallActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &AWallActor::OnBoxDestroyed);
}

void AWallActor::OnBoxDestroyed(AActor* DestroyedActor)
{
	// Should not be destroyed in game
	checkNoEntry();
}
