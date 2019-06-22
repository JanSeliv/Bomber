// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;

	// Find materials
	const TArray<TCHAR*> pathes{
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

AActor* AGeneratedMap::AddActorOnMap(const FCell& cell, EActorTypeEnum actorType)
{
	const TSubclassOf<AActor> ACTOR_CLASS = *typesByClassesMap_.Find(actorType);
	UChildActorComponent* ChildActor = NewObject<UChildActorComponent>(this);
	ChildActor->SetupAttachment(GetRootComponent());
	ChildActor->bEditableWhenInherited = true;
	ChildActor->RegisterComponent();
	ChildActor->SetChildActorClass(ACTOR_CLASS);
	ChildActor->CreateChildActor();

	AddActorOnMapByObj(cell, ChildActor->GetChildActor());
	return ChildActor->GetChildActor();
}

void AGeneratedMap::AddActorOnMapByObj(const FCell& cell, AActor* updateActor)
{
	if (ISVALID(updateActor) == false			   // Updating actor is not valid
		|| GeneratedMap_.Contains(cell) == false)  // Not existing cell
	{
		return;
	}

	// Add actor to specific array
	const ACharacter* updateCharacter = Cast<ACharacter>(updateActor);
	if (updateCharacter != nullptr)  // if it is character, add to array of characters
	{
		charactersOnMap_.Add(updateCharacter);  // Add this character
	}
	else  // else if this class can be added
		if (typesByClassesMap_.FindKey(updateActor->GetClass()) != nullptr)
	{
		const FCell* cellOfExistingActor = GeneratedMap_.FindKey(updateActor);
		if (cellOfExistingActor != nullptr && cellOfExistingActor->location != cell.location)
		{
			GeneratedMap_.Add(*cellOfExistingActor);  // remove this actor from previous cell
			UE_LOG_STR("AddActorOnMapByObj: %s was existed", updateActor);
		}
		GeneratedMap_.Add(cell, updateActor);  // Add this actor to his cell
	}

	updateActor->GetRootComponent()->SetAbsolute(true, true, true);

	// Set actor on cell
	const FRotator ROTATOR = FRotator(0.f, FMath::RandRange(int32(0), int32(3)) * 90.f, 0.f);
	const FVector SCALE = FVector(1.f, 1.f, 1.f);
	updateActor->SetActorTransform(FTransform(ROTATOR, cell.location, SCALE));

	// Attach nongenerated dragged actor
	if (updateActor->IsChildActor() == false)
	{
		updateActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	UE_LOG_STR("AddActorOnMapByObj: %s ADDED", updateActor);
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

	UE_LOG_STR("AGeneratedMap::BeginPlay: %s", this);
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
	TArray<AActor*> childActors;
	GetAllChildActors(childActors);

	for (int32 i = childActors.Num() - 1; i >= 0; --i)
	{
		childActors[i]->Destroy();
	}

	GeneratedMap_.Empty();
	charactersOnMap_.Empty();
}
