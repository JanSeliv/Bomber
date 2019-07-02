// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "ConstructorHelpers.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "SingletonLibrary.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;

	// Initialize root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetRelativeScale3D(FVector(5.f, 5.f, 1.f));
	RootComponent->SetMobility(EComponentMobility::Static);

	// Find blueprints
	const TArray<TCHAR*> Paths{
		TEXT("/Game/Bomber/Assets/Wall"),		  //EPathTypesEnum::Wall
		TEXT("/Game/Bomber/Assets/Box"),		  //EPathTypesEnum::Box
		TEXT("/Game/Bomber/Blueprints/BpBomb"),   //EPathTypesEnum::Bomb
		TEXT("/Game/Bomber/Blueprints/BpItem"),   //EPathTypesEnum::Item
		TEXT("/Game/Bomber/Blueprints/BpPlayer")  //EPathTypesEnum::Player
	};
	for (int32 i = 0; i < Paths.Num(); ++i)
	{
		ConstructorHelpers::FClassFinder<AActor> ClassFinder(Paths[i]);
		TypesByClassesMap_.Add(
			EActorTypeEnum(1 << i), (ClassFinder.Succeeded() ? ClassFinder.Class : nullptr));
	}
}

TSet<FCell> AGeneratedMap::IntersectionCellsByTypes_Implementation(
	const TSet<FCell>& Cells,
	const uint8 ActorsTypesBitmask,
	const ACharacter* ExcludePlayer) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(
	const FCell& Cell,
	const int32 SideLength,
	const EPathTypesEnum Pathfinder) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
}

AActor* AGeneratedMap::AddActorOnMap(const FTransform& Transform, const EActorTypeEnum ActorType)
{
	const TSubclassOf<AActor> ActorClass = *TypesByClassesMap_.Find(ActorType);
	if (ActorClass == nullptr	  // is not valid class for generation
		|| GetWorld() == nullptr)  // World is null
	{
		return nullptr;
	}

	AActor* const SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform);
	return SpawnedActor;
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
		const FCell* CellOfExistingActor = GridArray_.FindKey(UpdateActor);
		if (CellOfExistingActor != nullptr && CellOfExistingActor->Location != Cell.Location)
		{
			GridArray_.Add(*CellOfExistingActor);  // remove this actor from previous cell
			UE_LOG_STR("AddActorOnMapByObj: %s was existed", UpdateActor);
		}
		GridArray_.Add(Cell, UpdateActor);  // Add this actor to his cell
	}

	UpdateActor->GetRootComponent()->SetAbsolute(true, true, true);

	// Locate actor on cell
	const FRotator Rotation{0.f, FMath::RandRange(int32(0), int32(3)) * 90.f, 0.f};
	const FVector Translation{Cell.Location.X, Cell.Location.Y, Cell.Location.Z + 100.f};
	const FVector Scale{1.f, 1.f, 1.f};
	UpdateActor->SetActorTransform(FTransform(Rotation, Translation, Scale));

	// Attach to the Level Map actor
	UpdateActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	UE_LOG(LogTemp, Warning, TEXT("AddActorOnMapByObj: %s ADDED (%s)"), *UpdateActor->GetName(), *Cell.Location.ToString());
}

void AGeneratedMap::DestroyActorsFromMap_Implementation(const TSet<FCell>& Keys)
{
	UE_LOG(LogTemp, Warning, TEXT("DestroyActorsFromMap for %i keys"), Keys.Num());
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::GetSingleton()->LevelMap_ = this;

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
	//Destroy all attached actors and remove all elements of arrays
	ClearLevelMap();

	// Clear Singleton Level Map obj
	USingletonLibrary::GetSingleton()->LevelMap_ = nullptr;

	Super::Destroyed();
}

void AGeneratedMap::ClearLevelMap()
{
	// Destroy all attached actors
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	if (AttachedActors.Num() > 0)
	{
		for (int32 i = AttachedActors.Num() - 1; i >= 0; --i)
		{
			AttachedActors[i]->Destroy();
		}
		UE_LOG(LogTemp, Warning, TEXT("ClearLevelMap: %i components left"), AttachedActors.Num());
	}

	// Remove all elements of arrays
	GridArray_.Empty();
	CharactersOnMap_.Empty();
}

void AGeneratedMap::GenerateLevelMap_Implementation()
{
	ClearLevelMap();
}
