// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BoxActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "DataAssets/BoxDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Math/UnrealMathUtility.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(BoxActor)

// Sets default values.
ABoxActor::ABoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	static constexpr float NewNewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewNewUpdateFrequency);
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
}

// Initialize a box actor, could be called multiple times
void ABoxActor::ConstructBoxActor()
{
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionBoxActor);
	MapComponentInternal->ConstructOwnerActor();
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
	constexpr int32 MaxChance = 100;
	const int32 CurrentChance = FMath::RandHelper(MaxChance);
	const int32 PowerupsChance = UBoxDataAsset::Get().GetPowerupsChance();
	if (CurrentChance <= PowerupsChance)
	{
		AGeneratedMap::Get().SpawnActorByType(EAT::Item, MapComponentInternal->GetCell());
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned.
void ABoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructBoxActor();
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
}

void ABoxActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (!bNewHidden)
	{
		// Is added on Generated Map
		ConstructBoxActor();
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called on a box actor construction, could be called multiple times
void ABoxActor::OnConstructionBoxActor_Implementation()
{
	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	// Implement here any logic on spawn this actor
}

// Called when owned map component is destroyed on the Generated Map
void ABoxActor::OnDeactivatedMapComponent_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	const bool bIsCauserAllowedForItems = UMyBlueprintFunctionLibrary::IsActorHasAnyMatchingType(Cast<AActor>(DestroyCauser), TO_FLAG(EAT::Bomb | EActorType::Player));
	if (bIsCauserAllowedForItems)
	{
		TrySpawnItem();
	}
}