// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/ItemActor.h"

// Bomber
#include "Components/MapComponent.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/ItemDataAsset.h"
#include "GeneratedMap.h"
#include "Structures/BmrGameplayTags.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

// UE
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemActor)

// Sets default values
AItemActor::AItemActor()
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

// Set new item type, can be called on the server-only
void AItemActor::SetItemType(FBmrPowerupTag NewItemType)
{
	if (!HasAuthority()
	    || ItemTypeInternal == NewItemType)
	{
		return;
	}

	ItemTypeInternal = NewItemType;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemTypeInternal, this);
}

// Is called on client when item type is replicated
void AItemActor::OnRep_ItemType()
{
	UpdateItemMesh();
}

// Is called on both server and clients to update the item mesh based on the item type
void AItemActor::UpdateItemMesh()
{
	if (const UItemRow* FoundItemRow = UItemDataAsset::Get().GetRowByItemType(ItemTypeInternal, UMyBlueprintFunctionLibrary::GetLevelType()))
	{
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		MapComponentInternal->SetLocalMesh(FoundItemRow->Mesh);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	AGeneratedMap::Get().AddToGrid(MapComponentInternal);
}

// Returns properties that are replicated for the lifetime of the actor channel
void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemTypeInternal, Params);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void AItemActor::OnAddedToLevel_Implementation(UMapComponent* MapComponent)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	OnActorBeginOverlap.AddUniqueDynamic(this, &AItemActor::OnItemBeginOverlap);

	// Rand the item type if not set yet
	if (HasAuthority()
	    && ItemTypeInternal == FBmrPowerupTag::None)
	{
		const int32 RandomIndex = FMath::RandRange(0, FBmrPowerupTag::GetAll().Num() - 1);
		const FBmrPowerupTag NewItemType = FBmrPowerupTag::GetAll().GetByIndex(RandomIndex);
		SetItemType(NewItemType);
		UpdateItemMesh();
	}
}

// Triggers when this item starts overlap a player character to destroy itself
void AItemActor::OnItemBeginOverlap_Implementation(AActor* OverlappedActor, AActor* OtherActor)
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
	EventData.InstigatorTags.AddTag(ItemTypeInternal);
	ASC->HandleGameplayEvent(BmrGameplayTags::Event::Powerup_Collected, &EventData);
}

// Called when this level actor is destroyed from the Generated Map
void AItemActor::OnPostRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	SetItemType(FBmrPowerupTag::None);
}