// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrBoxActor.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrBoxDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "Subsystems/BmrGlobalEventsSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Math/UnrealMathUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBoxActor)

// Sets default values.
ABmrBoxActor::ABmrBoxActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	static constexpr float NewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewUpdateFrequency);
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UBmrMapComponent>(TEXT("MapComponent"));
}

// Spawn powerup with a chance
void ABmrBoxActor::TrySpawnPowerup()
{
	if (!HasAuthority()
	    || ABmrGameState::GetCurrentGameState() != EBmrCurrentGameState::InGame)
	{
		return;
	}

	constexpr int32 MaxChance = 100;
	const int32 CurrentChance = FMath::RandHelper(MaxChance);
	const int32 PowerupsChance = UBmrBoxDataAsset::Get().GetPowerupsChance();
	if (CurrentChance <= PowerupsChance)
	{
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		ABmrGeneratedMap::Get().SpawnActorByType(EAT::Powerup, MapComponent->GetCell());
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned.
void ABmrBoxActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	ABmrGeneratedMap::Get().AddToGrid(MapComponent);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void ABmrBoxActor::OnAddedToLevel_Implementation(UBmrMapComponent* InMapComponent)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);
}

// Called when this level actor is destroyed on the Generated Map
void ABmrBoxActor::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	const bool bIsCauserAllowed = UBmrBlueprintFunctionLibrary::IsActorHasAnyMatchingType(Cast<AActor>(DestroyCauser), TO_FLAG(EAT::Bomb | EBmrActorType::Player));
	if (bIsCauserAllowed)
	{
		TrySpawnPowerup();
	}
}