// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/ItemActor.h"
//---
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
//---
#include "Bomber.h"
#include "LevelActors/PlayerCharacter.h"
#include "MapComponent.h"

// Default constructor
UItemDataAsset::UItemDataAsset()
{
	ActorTypeInternal = EAT::Item;
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

	// Initialize item mesh component
	ItemMeshComponentInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMeshComponentInternal->SetupAttachment(RootComponent);
}

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)			// This actor is transient
		|| !IsValid(MapComponentInternal))	// Is not valid for map construction
	{
		return;
	}

	// Rand the item type if not set yet
	if (ItemTypeInternal == EItemType::None)
	{
		if (const static UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EItemType"), true))
		{
			ItemTypeInternal = TO_ENUM(EItemType, Enum->GetValueByIndex(FMath::RandRange(1, TO_FLAG(Enum->GetMaxEnumValue() - 1))));
		}
	}

	// Construct the actor's map component
	MapComponentInternal->OnComponentConstruct(ItemMeshComponentInternal, FLevelActorMeshRow(ItemTypeInternal));
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
	if (!IS_VALID(this)						// Other actor is not myCharacter
		|| !IsValid(OverlappedCharacter))	// Character is not valid
	{
		return;
	}

	// Destroy itself on overlapping
	this->Destroy();
}
