// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "GameFramework/Character.h"
// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Shouldt call OnConsturction on drag events
	bRunConstructionScriptOnDrag = false;
}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const
{
	TSet<FCell> foundedLocations;
	return foundedLocations;
}

TSet<FCell> AGeneratedMap::FilterCellsByTypes_Implementation(const TSet<FCell>& keys, TArray<EActorTypeEnum>& filterTypes, const ACharacter* excludePlayer) const
{
	TSet<FCell> foundedLocations;
	return foundedLocations;
}

AActor* AGeneratedMap::AddActorOnMap_Implementation(const FCell& cell, EActorTypeEnum actorType)
{
	return nullptr;
}

void AGeneratedMap::AddActorOnMapByObj_Implementation(const FCell& cell, const AActor* updateActor)
{
	if (ISVALID(updateActor) == false || !GeneratedMap_.Contains(cell) || ISTRANSIENT(updateActor))
		return;

	// Add to specific array
	const ACharacter* updateCharacter = Cast<ACharacter>(updateActor);
	if (updateCharacter != nullptr)
	{
		charactersOnMap_.Add(updateCharacter);  // Add this character
	}
	else
	{
		const FCell* cellOfExistingActor = GeneratedMap_.FindKey(updateActor);
		if (cellOfExistingActor != nullptr && cellOfExistingActor->location != cell.location)
		{
			GeneratedMap_.Add(*cellOfExistingActor);  // remove this actor from previous cell
			UE_LOG_STR("AddActorOnMapByObj: %s was existed", *updateActor->GetFName().ToString());
		}
		GeneratedMap_.Add(cell, updateActor);  // Add this actor to his cell
	}
}

void AGeneratedMap::DestroyActorsFromMap_Implementation(const FCell& cell)
{
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::SetLevelMap(this);

	// fix null keys
	USingletonLibrary::GetLevelMap()->charactersOnMap_.Compact();
	USingletonLibrary::GetLevelMap()->charactersOnMap_.Shrink();

	//onActorsUpdatedDelegate.Broadcast();
	UE_LOG_STR("AGeneratedMap::BeginPlay: %s", *this->GetFullName());
}

void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	if (ISTRANSIENT(this) == true)
	{
		return;
	}
	//Regenerate map;
	GenerateLevelMap();
}

void AGeneratedMap::Destroyed()
{
	// Destroying attached actors
	TArray<AActor*> attachedActors;
	GetAttachedActors(attachedActors);
	for (AActor* attachedActor : attachedActors)
	{
		attachedActor->Destroy();
	}

	Super::Destroyed();
}

void AGeneratedMap::GenerateLevelMap_Implementation()
{
	// Update LevelMap obj before generating child actors;
	USingletonLibrary::SetLevelMap(this);

	GeneratedMap_.Empty();
	charactersOnMap_.Empty();
}
