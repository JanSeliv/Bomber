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
	const TArray<TCHAR*> Pathes{
		TEXT("/Game/Bomber/Assets/Wall"),			//EPathTypesEnum::Wall
		TEXT("/Game/Bomber/Assets/Box"),			//EPathTypesEnum::Box
		TEXT("/Game/Bomber/Blueprints/BpBomb"),		//EPathTypesEnum::Bomb
		TEXT("/Game/Bomber/Blueprints/BpItem"),		//EPathTypesEnum::Item
		TEXT("/Game/Bomber/Blueprints/BpPlayer")};  //EPathTypesEnum::Player
	for (int32 i = 0; i < Pathes.Num(); ++i)
	{
		ConstructorHelpers::FClassFinder<AActor> ClassFinder(Pathes[i]);
		TypesByClassesMap_.Add(
			EActorTypeEnum(1 << i), (ClassFinder.Succeeded() ? ClassFinder.Class : nullptr));
	}
}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(const FCell& Cell, int32 SideLength, EPathTypesEnum Pathfinder) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
}

TSet<FCell> AGeneratedMap::FilterCellsByTypes_Implementation(const TSet<FCell>& Keys, const EActorTypeEnum& FilterTypes, const ACharacter* excludePlayer) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
}

AActor* AGeneratedMap::AddActorOnMap(const FCell& Cell, const EActorTypeEnum& ActorType)
{
	const TSubclassOf<AActor> ActorClass = *TypesByClassesMap_.Find(ActorType);
	UChildActorComponent* ChildActorComponent = NewObject<UChildActorComponent>(this);
	ChildActorComponent->SetupAttachment(GetRootComponent());
	ChildActorComponent->bEditableWhenInherited = true;
	ChildActorComponent->RegisterComponent();
	ChildActorComponent->SetChildActorClass(ActorClass);
	ChildActorComponent->CreateChildActor();

	AddActorOnMapByObj(Cell, ChildActorComponent->GetChildActor());
	return ChildActorComponent->GetChildActor();
}

void AGeneratedMap::AddActorOnMapByObj(const FCell& Cell, AActor* UpdateActor)
{
	if (IS_VALID(UpdateActor) == false			// Updating actor is not valid
		|| GridArray_.Contains(Cell) == false)  // Not existing cell
	{
		return;
	}

	// Add actor to specific array
	const ACharacter* UpdateCharacter = Cast<ACharacter>(UpdateActor);
	if (UpdateCharacter != nullptr)  // if it is character, add to array of characters
	{
		CharactersOnMap_.Add(UpdateCharacter);  // Add this character
	}
	else  // else if this class can be added
		if (TypesByClassesMap_.FindKey(UpdateActor->GetClass()) != nullptr)
	{
		const FCell* cellOfExistingActor = GridArray_.FindKey(UpdateActor);
		if (cellOfExistingActor != nullptr && cellOfExistingActor->Location != Cell.Location)
		{
			GridArray_.Add(*cellOfExistingActor);  // remove this actor from previous cell
			UE_LOG_STR("AddActorOnMapByObj: %s was existed", UpdateActor);
		}
		GridArray_.Add(Cell, UpdateActor);  // Add this actor to his cell
	}

	UpdateActor->GetRootComponent()->SetAbsolute(true, true, true);

	// Locate actor on cell
	const FRotator Rotator{0.f, FMath::RandRange(int32(0), int32(3)) * 90.f, 0.f};
	const FVector Scale{1.f, 1.f, 1.f};
	UpdateActor->SetActorTransform(FTransform(Rotator, Cell.Location, Scale));

	// Attach non generated dragged actor
	if (UpdateActor->IsChildActor() == false)
	{
		UpdateActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	UE_LOG_STR("AddActorOnMapByObj: %s ADDED", UpdateActor);
}

void AGeneratedMap::DestroyActorsFromMap_Implementation(const TSet<FCell>& Keys)
{
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::GetSingleton()->levelMap_ = this;

	// fix null keys
	CharactersOnMap_.Compact();
	CharactersOnMap_.Shrink();

	UE_LOG_STR("AGeneratedMap::BeginPlay: %s", this);
}

void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	if (IS_TRANSIENT(this) == true)
	{
		return;
	}
	//Regenerate map;
	GenerateLevelMap();
}

void AGeneratedMap::Destroyed()
{
	// Destroying attached actors
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		AttachedActor->Destroy();
	}

	Super::Destroyed();
}

void AGeneratedMap::GenerateLevelMap_Implementation()
{
	TArray<AActor*> ChildActors;
	GetAllChildActors(ChildActors);

	for (int32 i = ChildActors.Num() - 1; i >= 0; --i)
	{
		ChildActors[i]->Destroy();
	}

	GridArray_.Empty();
	CharactersOnMap_.Empty();
}
