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
	mapComponent_ = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

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
	if (ISVALID(mapComponent_) == false || ISTRANSIENT(this) == true)
	{
		return;
	}
	mapComponent_->UpdateSelfOnMap();
}

void AItem::OnItemBeginOverlap(AActor* overlappedItem, AActor* otherActor)
{
	if (overlappedItem == this || Cast<AMyCharacter>(otherActor) == nullptr)
	{
		return;
	}

	AMyCharacter* character = Cast<AMyCharacter>(otherActor);
	character->powerups_;
}
