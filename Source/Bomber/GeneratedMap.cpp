// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"
#include "Bomber.h"

FCell::FCell(const AActor* actor)
{
	CHECKMAP();
	if (IsValid(actor) == false) return;
	this->location = actor->GetActorLocation();

	if (USingletonLibrary::GetLevelMap()->GeneratedMap_.Num() == 0) return;

	// Already exist?
	if (USingletonLibrary::GetLevelMap()->GeneratedMap_.Contains(*this))
	{
		return;
	}

	FCell foundedCell;
	for (auto i : USingletonLibrary::GetLevelMap()->GeneratedMap_)
	{
		if (USingletonLibrary::CalculateCellsLength(i.Key, *this)
			< USingletonLibrary::CalculateCellsLength(foundedCell, *this))
		{
			foundedCell.location = i.Key.location;
		}
	}
	this->location = foundedCell.location;
}


// Sets default values
AGeneratedMap::AGeneratedMap()
{

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

AActor* AGeneratedMap::AddActorOnMap_Implementation(const FCell& cell, EActorTypeEnum actorType)
{

	return nullptr;
}

void AGeneratedMap::AddActorOnMapByObj_Implementation(const AActor* updateActor)
{

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

bool AGeneratedMap::GenerateLevelMap_Implementation()
{
	if (IsValid(USingletonLibrary::GetSingleton()) == false) return false;
	if (IsValid(USingletonLibrary::GetLevelMap()) == false)
	{
		USingletonLibrary::GetSingleton()->levelMap_ = this;
	}
	GeneratedMap_.Empty();
	charactersOnMap_.Empty();

	return true;
}
