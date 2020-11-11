// Copyright 2020 Yevhenii Selivanov.

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

	// Rand the item type if not set yet
	if (ItemTypeInternal == EItemType::None)
	{
		if (const static UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EItemType"), true))
		{
			ItemTypeInternal = TO_ENUM(EItemType, Enum->GetValueByIndex(FMath::RandRange(1, TO_FLAG(Enum->GetMaxEnumValue() - 1))));
		}
	}

	// Override mesh
	TArray<ULevelActorRow*> OutRows;
	MapComponentInternal->GetDataAssetChecked<UItemDataAsset>()->GetRowsByLevelType(OutRows, TO_FLAG(USingletonLibrary::GetLevelType()));
	const auto FoundRow = OutRows.FindByPredicate([ItemType = ItemTypeInternal](const ULevelActorRow* RowIt)
	{
		const auto ItemRow = Cast<UItemRow>(RowIt);
		return ItemRow && ItemRow->ItemType == ItemType;
	});
	ULevelActorRow* LevelActorRow = FoundRow ? *FoundRow : nullptr;
	if (LevelActorRow)
	{
		MapComponentInternal->SetMesh(LevelActorRow->Mesh);
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
	if (AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap())
	{
		LevelMap->DestroyLevelActor(MapComponentInternal);
	}
}
