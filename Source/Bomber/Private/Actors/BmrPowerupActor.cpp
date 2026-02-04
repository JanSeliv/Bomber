// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrPowerupActor.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrDataAssetsContainer.h"
#include "DataAssets/BmrPowerupDataAsset.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGameplayMessageSubsystem.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPowerupActor)

// Sets default values
ABmrPowerupActor::ABmrPowerupActor()
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

// Set new Powerup type, can be called on the server-only
void ABmrPowerupActor::SetPowerupTag(FBmrPowerupTag InPowerupTag)
{
	if (!HasAuthority()
	    || PowerupTag == InPowerupTag)
	{
		return;
	}

	PowerupTag = InPowerupTag;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PowerupTag, this);
}

// Is called on client when Powerup type is replicated
void ABmrPowerupActor::OnRep_PowerupTag()
{
	UpdatePowerupMesh();
}

// Is called on both server and clients to update the Powerup mesh based on the Powerup type
void ABmrPowerupActor::UpdatePowerupMesh()
{
	if (const UBmrPowerupRow* FoundPowerupRow = UBmrPowerupDataAsset::Get().GetRowByPowerupTag(PowerupTag))
	{
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		MapComponent->SetLocalMesh(FoundPowerupRow->Mesh);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned
void ABmrPowerupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	ABmrGeneratedMap::Get().AddToGrid(MapComponent);
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABmrPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PowerupTag, Params);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void ABmrPowerupActor::OnAddedToLevel_Implementation(UBmrMapComponent* InMapComponent)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	OnActorBeginOverlap.AddUniqueDynamic(this, &ABmrPowerupActor::OnPowerupBeginOverlap);

	// Rand the Powerup type if not set yet
	if (HasAuthority()
	    && PowerupTag == FBmrPowerupTag::None)
	{
		const int32 RandomIndex = FMath::RandRange(0, FBmrPowerupTag::GetAll().Num() - 1);
		const FBmrPowerupTag InPowerupTag = FBmrPowerupTag::GetAll().GetByIndex(RandomIndex);
		SetPowerupTag(InPowerupTag);
		UpdatePowerupMesh();
	}
}

// Triggers when this Powerup starts overlap a player character to destroy itself
void ABmrPowerupActor::OnPowerupBeginOverlap_Implementation(AActor* OverlappedActor, AActor* OtherActor)
{
	if (IsHidden())
	{
		// Might happen on predicted client when the Powerup is already collected
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = BmrGameplayTags::Event::Powerup_Collected;
	EventData.Instigator = this;
	EventData.Target = OtherActor;
	EventData.InstigatorTags.AddTag(PowerupTag);
	UBmrGameplayMessageSubsystem::BroadcastMessage(EventData);
}

// Called when this level actor is destroyed from the Generated Map
void ABmrPowerupActor::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	SetPowerupTag(FBmrPowerupTag::None);
}