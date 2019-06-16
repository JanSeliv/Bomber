// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "Bomber.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>("Map Component");
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ISVALID(mapComponent) == false || ISTRANSIENT(this) == true)
	{
		return;
	}
	mapComponent->UpdateSelfOnMap();
}