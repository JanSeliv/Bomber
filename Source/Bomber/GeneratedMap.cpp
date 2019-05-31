// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"
#include "Bomber.h"

FCell::FCell()
{

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


bool AGeneratedMap::GenerateLevelMap_Implementation()
{
	if (IsValid(USingletonLibrary::GetSingleton()) == false) return false;
	if (IsValid(USingletonLibrary::GetSingleton()->levelMap) == false)
	{
		USingletonLibrary::GetSingleton()->levelMap = this;
	}
	GeneratedMap_.Empty();

	return true;
}


void AGeneratedMap::ShowTMap(FCell cell, int cellNo)
{
	PRINT(
		FString::FromInt(cellNo) + " "
		+ (cell.location).ToString()
		+ (IsValid(mapActor) ? mapActor->GetFullName() : "nullptr")
	);

}