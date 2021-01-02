// Copyright 2021 Yevhenii Selivanov.

#include "LevelActors/BoxActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
//---
#include "Math/UnrealMathUtility.h"

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
}

// Called when an instance of this class is placed (in editor) or spawned.
void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component.
	MapComponentInternal->OnConstruction();
}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABoxActor::TrySpawnItem);

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}

	ResetItemChance();
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void ABoxActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (bNewHidden)
	{
		TrySpawnItem();
	}
}


// Spawn item with a chance
void ABoxActor::TrySpawnItem(AActor* DestroyedActor/* = nullptr*/)
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap                         // The Level Map is not valid or transient (in regenerating process)
	    || !IsValid(MapComponentInternal) // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState(this) != ECurrentGameState::InGame)
	{
		return;
	}

	// Spawn item with the chance
	const int32 Max = 100;
	if (FMath::RandHelper(Max) < SpawnItemChanceInternal)
	{
		USingletonLibrary::PrintToLog(this, "OnBoxDestroyed", "Item will be spawned");
		LevelMap->SpawnActorByType(EAT::Item, FCell(GetActorLocation()));
	}
}

// The item chance can be overrided in game, so it should be reset for each new game
void ABoxActor::ResetItemChance()
{
	// Update current chance from Data Asset
	if (MapComponentInternal)
	{
		SpawnItemChanceInternal = MapComponentInternal->GetDataAssetChecked<UBoxDataAsset>()->GetSpawnItemChance();
	}
}

// Listen to reset item chance for each new game
void ABoxActor::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::GameStarting)
	{
		// Update current chance from Data Asset
		ResetItemChance();
	}
}
