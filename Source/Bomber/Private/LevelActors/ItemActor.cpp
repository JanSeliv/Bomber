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
	ActorTypeInternal = AT::Item;
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
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize item mesh component
	ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMeshComponent->SetupAttachment(RootComponent);
	ItemMeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	// Find and fill item meshes array
	static TArray<ConstructorHelpers::FObjectFinder<UStaticMesh>> ItemMeshFinderArray{
		TEXT("/Game/Bomber/Meshes/SM_Item_Skate"),
		TEXT("/Game/Bomber/Meshes/SM_Item_Bomb"),
		TEXT("/Game/Bomber/Meshes/SM_Item_Fire")};
	for (int32 i = 0; i < ItemMeshFinderArray.Num(); ++i)
	{
		if (ItemMeshFinderArray[i].Succeeded())
		{
			ItemTypesByMeshes.Emplace(static_cast<EItemType>(i + 1), ItemMeshFinderArray[i].Object);
			if (i == 0) ItemMeshComponent->SetStaticMesh(ItemMeshFinderArray[i].Object);  // Preview
		}
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)			// This actor is transient
		|| !IsValid(MapComponent))	// Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

	// Rand the item type
	if (ItemTypeInternal == EItemType::None)
	{
		TArray<EItemType> ItemTypesArray;
		ItemTypesByMeshes.GetKeys(ItemTypesArray);
		const int32 RandItemTypeNo = FMath::RandRange(0, ItemTypesArray.Num() - 1);
		ItemTypeInternal = ItemTypesArray[RandItemTypeNo];
	}

	UStaticMesh* FoundMesh = *ItemTypesByMeshes.Find(ItemTypeInternal);
	ensureAlwaysMsgf(FoundMesh != nullptr, TEXT("The item mesh of this type was not found"));
	ItemMeshComponent->SetStaticMesh(FoundMesh);
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
