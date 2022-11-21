// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BoxActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "Math/UnrealMathUtility.h"

// Default constructor
UBoxDataAsset::UBoxDataAsset()
{
	ActorTypeInternal = EAT::Box;
}

// Returns the box data asset
const UBoxDataAsset& UBoxDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Box);
	const auto BoxDataAsset = Cast<UBoxDataAsset>(FoundDataAsset);
	checkf(BoxDataAsset, TEXT("The Box Data Asset is not valid"));
	return *BoxDataAsset;
}

// Sets default values.
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	NetUpdateFrequency = 10.f;
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionBoxActor);
}

// Initialize a box actor, could be called multiple times
void ABoxActor::ConstructBoxActor()
{
	if (IsValid(MapComponentInternal))
	{
		MapComponentInternal->ConstructOwnerActor();
	}
}

// Called when an instance of this class is placed (in editor) or spawned.
void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructBoxActor();
}

// Is called on a box actor construction, could be called multiple times
void ABoxActor::OnConstructionBoxActor()
{
	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	UpdateItemChance();
}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		check(MapComponentInternal);
		MapComponentInternal->OnDeactivatedMapComponent.AddDynamic(this, &ThisClass::OnDeactivatedMapComponent);
	}

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

void ABoxActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (!bNewHidden)
	{
		// Is added on level map
		ConstructBoxActor();
	}
}

// Called when owned map component is destroyed on the level map
void ABoxActor::OnDeactivatedMapComponent(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	const bool bIsCauserAllowedForItems = USingletonLibrary::IsActorHasAnyMatchingType(Cast<AActor>(DestroyCauser), TO_FLAG(EAT::Bomb | EActorType::Player));
	if (bIsCauserAllowedForItems)
	{
		TrySpawnItem();
	}
}

// Spawn item with a chance
void ABoxActor::TrySpawnItem()
{
	if (!IsValid(MapComponentInternal) // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState() != ECurrentGameState::InGame)
	{
		return;
	}

	// Spawn item with the chance
	static constexpr int32 Max = 100;
	if (FMath::RandHelper(Max) < SpawnItemChanceInternal)
	{
		AGeneratedMap::Get().SpawnActorByType(EAT::Item, MapComponentInternal->GetCell());
	}
}

// The item chance can be overrided in game, so it should be reset for each new game
void ABoxActor::UpdateItemChance()
{
	// Update current chance from Data Asset
	if (MapComponentInternal)
	{
		SpawnItemChanceInternal = UBoxDataAsset::Get().GetSpawnItemChance();
	}
}

// Listen to reset item chance for each new game
void ABoxActor::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::GameStarting)
	{
		// Update current chance from Data Asset
		UpdateItemChance();
	}
}
