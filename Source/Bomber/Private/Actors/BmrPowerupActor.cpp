// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrPowerupActor.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrDataAssetsContainer.h"
#include "DataAssets/BmrPowerupDataAsset.h"
#include "Structures/BmrGameplayTags.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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

// Set new item type, can be called on the server-only
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

// Is called on client when item type is replicated
void ABmrPowerupActor::OnRep_PowerupType()
{
	UpdateItemMesh();
}

// Is called on both server and clients to update the item mesh based on the item type
void ABmrPowerupActor::UpdateItemMesh()
{
	if (const UBmrPowerupRow* FoundItemRow = UBmrPowerupDataAsset::Get().GetRowByItemType(PowerupTag, UBmrBlueprintFunctionLibrary::GetLevelType()))
	{
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		MapComponent->SetLocalMesh(FoundItemRow->Mesh);
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

	OnActorBeginOverlap.AddUniqueDynamic(this, &ABmrPowerupActor::OnItemBeginOverlap);

	// Rand the item type if not set yet
	if (HasAuthority()
	    && PowerupTag == FBmrPowerupTag::None)
	{
		const int32 RandomIndex = FMath::RandRange(0, FBmrPowerupTag::GetAll().Num() - 1);
		const FBmrPowerupTag NewItemType = FBmrPowerupTag::GetAll().GetByIndex(RandomIndex);
		SetPowerupTag(NewItemType);
		UpdateItemMesh();
	}
}

// Triggers when this item starts overlap a player character to destroy itself
void ABmrPowerupActor::OnItemBeginOverlap_Implementation(AActor* OverlappedActor, AActor* OtherActor)
{
	if (IsHidden())
	{
		// Might happen on predicted client when the item is already collected
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	if (!ASC)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.InstigatorTags.AddTag(PowerupTag);
	ASC->HandleGameplayEvent(BmrGameplayTags::Event::Powerup_Collected, &EventData);
}

// Called when this level actor is destroyed from the Generated Map
void ABmrPowerupActor::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	SetPowerupTag(FBmrPowerupTag::None);
}