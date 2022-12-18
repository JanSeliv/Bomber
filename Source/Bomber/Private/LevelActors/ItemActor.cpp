﻿// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/ItemActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "SoundsManager.h"
#include "Components/MapComponent.h"
#include "Globals/DataAssetsContainer.h"
#include "Globals/ItemDataAsset.h"
#include "UtilityLibraries/SingletonLibrary.h"
//---
#include "Net/UnrealNetwork.h"

// Sets default values
AItemActor::AItemActor()
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
}

// Initialize an item actor, could be called multiple times
void AItemActor::ConstructItemActor()
{
	checkf(MapComponentInternal, TEXT("%s: 'MapComponentInternal' is null"), *FString(__FUNCTION__));
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionItemActor);
	MapComponentInternal->ConstructOwnerActor();
}

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructItemActor();
}

// Is called on an item actor construction, could be called multiple times
void AItemActor::OnConstructionItemActor()
{
	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	// Rand the item type if not set yet
	if (ItemTypeInternal == EItemType::None)
	{
		const static FString ItemTypeEnumPathName = TEXT("/Script/Bomber.EItemType");
		if (static const UEnum* Enum = UClass::TryFindTypeSlow<UEnum>(ItemTypeEnumPathName, EFindFirstObjectOptions::ExactClass))
		{
			ItemTypeInternal = TO_ENUM(EItemType, Enum->GetValueByIndex(FMath::RandRange(1, TO_FLAG(Enum->GetMaxEnumValue() - 1))));
		}
	}

	// Override mesh
	if (const UItemRow* FoundItemRow = UItemDataAsset::Get().GetRowByItemType(ItemTypeInternal, USingletonLibrary::GetLevelType()))
	{
		MapComponentInternal->SetLevelActorRow(FoundItemRow);
	}
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AItemActor::OnItemBeginOverlap);
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void AItemActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (!bNewHidden)
	{
		// Is added on level map
		ConstructItemActor();
		return;
	}

	// Is removed from level map
	ResetItemType();
}

// Returns properties that are replicated for the lifetime of the actor channel
void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemTypeInternal);
}

// Triggers when this item starts overlap a player character to destroy itself
void AItemActor::OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor
	    || !OtherActor->IsA(UDataAssetsContainer::GetActorClassByType(EAT::Player)))
	{
		return;
	}

	USoundsManager::Get().PlayItemPickUpSFX();

	// Destroy itself on overlapping
	AGeneratedMap::Get().DestroyLevelActor(MapComponentInternal, OtherActor);
}
