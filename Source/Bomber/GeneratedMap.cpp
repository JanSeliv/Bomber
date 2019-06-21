// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Shouldt call OnConsturction on drag events
	bRunConstructionScriptOnDrag = false;

	// Find materials
	const TArray<TCHAR*> pathes{
		TEXT("/Game/Bomber/Assets/floor"),			//EPathTypesEnum::Floor
		TEXT("/Game/Bomber/Assets/Wall"),			//EPathTypesEnum::Wall
		TEXT("/Game/Bomber/Assets/Box"),			//EPathTypesEnum::Box
		TEXT("/Game/Bomber/Blueprints/BpBomb"),		//EPathTypesEnum::Bomb
		TEXT("/Game/Bomber/Blueprints/BpItem"),		//EPathTypesEnum::Item
		TEXT("/Game/Bomber/Blueprints/BpPlayer")};  //EPathTypesEnum::Player
	for (int32 i = 0; i < pathes.Num(); ++i)
	{
		ConstructorHelpers::FClassFinder<AActor> classFinder(pathes[i]);
		typesByClassesMap_.Add(
			EActorTypeEnum(1 << i), (classFinder.Succeeded() ? classFinder.Class : nullptr));
	}
}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(const FCell& cell, int32 sideLength, EPathTypesEnum pathfinder) const
{
	TSet<FCell> foundedLocations;
	return foundedLocations;
}

TSet<FCell> AGeneratedMap::FilterCellsByTypes_Implementation(const TSet<FCell>& keys, const EActorTypeEnum& filterTypes, const ACharacter* excludePlayer) const
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
	if (ISVALID(updateActor) == false			   // Updating actor is not valid
		|| GeneratedMap_.Contains(cell) == false)  // Not existing cell
	{
		return;
	}

	// if it is character, add to array of characters
	const ACharacter* updateCharacter = Cast<ACharacter>(updateActor);
	if (updateCharacter != nullptr)
	{
		charactersOnMap_.Add(updateCharacter);  // Add this character
	}
	else  // else if it is not the floor, add to TMap
		if (~(int32)EActorTypeEnum::Floor & (int32)*typesByClassesMap_.FindKey(updateActor->GetClass()))
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

void AGeneratedMap::DestroyActorsFromMap_Implementation(const TSet<FCell>& keys)
{
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::GetSingleton()->levelMap_ = this;

	// fix null keys
	charactersOnMap_.Compact();
	charactersOnMap_.Shrink();

	UE_LOG_STR("AGeneratedMap::BeginPlay: %s", *this->GetName());
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
	GeneratedMap_.Empty();
	charactersOnMap_.Empty();
}
