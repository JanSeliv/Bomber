// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxActor.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "Math/UnrealMathUtility.h"
#include "SingletonLibrary.h"

// Sets default values
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize map component
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));
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
	if (GetWorld() == nullptr									   // World is null
		|| !IS_VALID(USingletonLibrary::GetLevelMap(GetWorld())))  // level map is not valid
	{
		return;
	}

	// Spawn item with the chance
	if (FMath::RandRange(int32(0), int32(30)) < 100)
	{
		// Spawn Item
		UE_LOG_STR("OnBoxDestroyed: %s spawning item", this);
		USingletonLibrary::GetLevelMap(GetWorld())
			->AddActorOnMap(GetActorTransform(), EActorTypeEnum::Item);
	}
}
