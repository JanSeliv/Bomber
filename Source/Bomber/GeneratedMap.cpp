// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"
#include "Bomber.h"

FCell::FCell(FVector vector)
{

}


// Sets default values
AGeneratedMap::AGeneratedMap()
{

	if (IsValid(USingletonLibrary::GetSingleton()))
	{
		USingletonLibrary::GetSingleton()->levelMap = this;
	}

}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(FCell cellLocation, int32 sideLength, EPathTypesEnum pathfinder) const
{

	TSet<FCell> foundedLocations;
	return foundedLocations;
}

AActor* AGeneratedMap::AddActorOnMap_Implementation(FCell cellLocation, AActor* updateActor, EActorTypeEnum actorType)
{

	return nullptr;
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

}
