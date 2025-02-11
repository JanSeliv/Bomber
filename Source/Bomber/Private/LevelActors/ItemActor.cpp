// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/ItemActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/ItemDataAsset.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Net/UnrealNetwork.h"
//---
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
void AItemActor::SetItemType(EItemType NewItemType)
{
	if (!HasAuthority()
	    || ItemTypeInternal == NewItemType)
	{
		return;
	}

	ItemTypeInternal = NewItemType;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemTypeInternal, this);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->OnAddedToLevel.AddUniqueDynamic(this, &ThisClass::OnAddedToLevel);
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
	if (ItemTypeInternal == EItemType::None)
	{
		const int32 RandomIndex = FMath::RandRange(EIT_FIRST_FLAG, EIT_LAST_FLAG);
		const EItemType NewItemType = static_cast<EItemType>(RandomIndex);
		SetItemType(NewItemType);
	}

	// Override mesh
	if (const UItemRow* FoundItemRow = UItemDataAsset::Get().GetRowByItemType(ItemTypeInternal, UMyBlueprintFunctionLibrary::GetLevelType()))
	{
		MapComponent->SetMesh(FoundItemRow->Mesh);
	}
}

// Triggers when this item starts overlap a player character to destroy itself
void AItemActor::OnItemBeginOverlap_Implementation(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor
	    || !OtherActor->IsA(UDataAssetsContainer::GetActorClassByType(EAT::Player)))
	{
		return;
	}

	USoundsSubsystem::Get().PlayItemPickUpSFX();

	// Destroy itself on overlapping
	AGeneratedMap::Get().DestroyLevelActor(MapComponentInternal, OtherActor);
}

// Called when this level actor is destroyed from the Generated Map
void AItemActor::OnPostRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	OnActorBeginOverlap.RemoveAll(this);

	SetItemType(EItemType::None);
}