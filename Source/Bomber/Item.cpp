// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "Bomber.h"
#include "MyCharacter.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

	OnActorBeginOverlap.AddDynamic(this, &AItem::OnItemBeginOverlap);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ISVALID(mapComponent) == false)
	{
		return;
	}
	mapComponent->UpdateSelfOnMap();
}

void AItem::OnItemBeginOverlap(AActor* overlappedItem, AActor* otherActor)
{
	if (overlappedItem == this						   // self triggering
		|| Cast<AMyCharacter>(otherActor) == nullptr)  // other actor is not myCharacter
	{
		return;
	}

	AMyCharacter* character = Cast<AMyCharacter>(otherActor);
	character->powerups_;
}
