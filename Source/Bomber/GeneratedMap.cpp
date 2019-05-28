// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"
#include "Bomber.h"

FCell::FCell()
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

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const
{

	TSet<FCell> foundedLocations;
	return foundedLocations;
}

TSet<FCell> AGeneratedMap::FilterCellsByTypes_Implementation(const TSet<FCell>& keys, const TArray<EActorTypeEnum>& filterTypes, const ACharacter* excludePlayer) const
{
	TSet<FCell> foundedLocations;
	return foundedLocations;
}

AActor* AGeneratedMap::AddActorOnMap_Implementation(const FCell& cell, AActor* updateActor, EActorTypeEnum actorType = EActorTypeEnum::None)
{

	return nullptr;
}

bool AGeneratedMap::DestroyActorFromMap_Implementation(const FCell& cell)
{
	return true;
}


// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

}

void AGeneratedMap::GenerateLevelMap_Implementation()
{

}
