// Copyright 2021 Yevhenii Selivanov.

#include "LevelActors/ItemActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
//---
#include "Components/BoxComponent.h"

// Default constructor
UItemDataAsset::UItemDataAsset()
{
	ActorTypeInternal = EAT::Item;
	RowClassInternal = UItemRow::StaticClass();
}

// Returns the item data asset
const UItemDataAsset& UItemDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = USingletonLibrary::GetDataAssetByActorType(EActorType::Item);
	const auto ItemDataAsset = Cast<UItemDataAsset>(FoundDataAsset);
	checkf(ItemDataAsset, TEXT("The Item Data Asset is not valid"));
	return *ItemDataAsset;
}

// Return row by specified item type
UItemRow* UItemDataAsset::GetRowByItemType(EItemType ItemType, ELevelType LevelType) const
{
	TArray<ULevelActorRow*> OutRows;
	UItemDataAsset::Get().GetRowsByLevelType(OutRows, TO_FLAG(LevelType));
	ULevelActorRow* const* FoundRowPtr = OutRows.FindByPredicate([ItemType](const ULevelActorRow* RowIt)
	{
		const auto ItemRow = Cast<UItemRow>(RowIt);
		return ItemRow && ItemRow->ItemType == ItemType;
	});
	return FoundRowPtr ? Cast<UItemRow>(*FoundRowPtr) : nullptr;
}

// Sets default values
AItemActor::AItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
}

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	// Construct the map component
	MapComponentInternal->OnConstruction();
	if (!IS_VALID(this))
	{
		return;
	}

	// Rand the item type if not set yet
	if (ItemTypeInternal == EItemType::None)
	{
		if (static const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EItemType"), true))
		{
			ItemTypeInternal = TO_ENUM(EItemType, Enum->GetValueByIndex(FMath::RandRange(1, TO_FLAG(Enum->GetMaxEnumValue() - 1))));
		}
	}

	// Override mesh
	if (const UItemRow* FoundItemRow = UItemDataAsset::Get().GetRowByItemType(ItemTypeInternal, USingletonLibrary::GetLevelType()))
	{
		MapComponentInternal->SetMeshByRow(FoundItemRow);
	}
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AItemActor::OnItemBeginOverlap);
}

// Triggers when this item starts overlap a player character to destroy itself
void AItemActor::OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	const auto OverlappedCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!IS_VALID(this)          // is not pending killed
	    || !OverlappedCharacter) // character is not valid)
	{
		return;
	}

	// Destroy itself on overlapping
	AGeneratedMap::Get().DestroyLevelActor(MapComponentInternal);
}
