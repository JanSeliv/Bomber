// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/BoxActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"
//---
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"

// Default constructor
UBoxDataAsset::UBoxDataAsset()
{
	ActorTypeInternal = EAT::Box;
}

// Sets default values.
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize box mesh component
	BoxMeshComponentInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMeshComponent"));
	BoxMeshComponentInternal->SetupAttachment(RootComponent);
}

// Called when an instance of this class is placed (in editor) or spawned.
void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(MapComponentInternal) == false)	 // this component is not valid for owner construction
	{
		return;
	}

	// Construct the actor's map component.
	MapComponentInternal->OnComponentConstruct(BoxMeshComponentInternal, FLevelActorMeshRow::Empty);
}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABoxActor::OnBoxDestroyed);
}

// Event triggered when the actor has been explicitly destroyed. With some chances spawns an item.
void ABoxActor::OnBoxDestroyed(AActor* DestroyedActor)
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (LevelMap                           // The Level Map is not valid or transient (in regenerating process)
	    || !IsValid(MapComponentInternal)) // The Map Component is not valid or is destroyed already
	{
		return;
	}

	// Spawn item with the chance
	const int32 SpawnItemChance = MapComponentInternal->GetDataAssetChecked<UBoxDataAsset>()->GetSpawnItemChance();
	const int32 Max = 100;
	if(FMath::RandHelper(Max) < SpawnItemChance)
	{
		USingletonLibrary::PrintToLog(this, "OnBoxDestroyed", "Item will be spawned");
		LevelMap->SpawnActorByType(EAT::Item, FCell(GetActorLocation()));
	}
}
