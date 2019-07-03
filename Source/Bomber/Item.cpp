// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

#include "Bomber.h"
#include "MapComponent.h"
#include "MyCharacter.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddUniqueDynamic(this, &AItem::OnItemBeginOverlap);
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_VALID(MapComponent) == false)
	{
		return;
	}

	// Update this actor
	MapComponent->UpdateSelfOnMap();
}

void AItem::OnItemBeginOverlap_Implementation(AActor* OverlappedItem, AActor* OtherActor)
{
	/*
	if (OtherActor == nullptr   //is the actor that triggered the event
		|| OtherActor == this)  //is not ourself
	{
		return;
	}

	AMyCharacter* const MyCharacter = Cast<AMyCharacter>(OtherActor);
	if (MyCharacter != nullptr)  // other actor is not myCharacter
	{
		MyCharacter->Powerups_;
	}
	*/
}
