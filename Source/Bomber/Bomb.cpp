// Fill out your copyright notice in the Description page of Project Settings.

#include "Bomb.h"
#include "Bomber.h"
#include "MyCharacter.h"

// Sets default values
ABomb::ABomb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize mapComponent
	mapComponent = CreateDefaultSubobject<UMapComponent>("Map Component");
}

// Called when the game starts or when spawned
void ABomb::BeginPlay()
{
	Super::BeginPlay();
}

void ABomb::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ISVALID(mapComponent) == false || ISTRANSIENT(this) == true)
	{
		return;
	}
	mapComponent->UpdateSelfOnMap();
}

void ABomb::Destroyed()
{

	Super::Destroyed();
}

void ABomb::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
}
