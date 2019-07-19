// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

#include "Bomber.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MapComponent.h"
#include "MyCharacter.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize item mesh
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);

	// Find and fill item meshes array
	static TArray<ConstructorHelpers::FObjectFinder<UStaticMesh>> ItemMeshFinderArray{
		TEXT("/Game/Bomber/Assets/Meshes/SkateItemMesh"),
		TEXT("/Game/Bomber/Assets/Meshes/BombItemMesh"),
		TEXT("/Game/Bomber/Assets/Meshes/FireItemMesh")};
	for (int32 i = 0; i < ItemMeshFinderArray.Num(); ++i)
	{
		if (ItemMeshFinderArray[i].Succeeded())
		{
			ItemTypesByMeshes.Add(EItemTypeEnum(1 << i), ItemMeshFinderArray[i].Object);
		}
	}

	// Initialize Item Collision Component
	ItemCollisionComponent = CreateDefaultSubobject<UBoxComponent>("ItemCollisionComponent");
	ItemCollisionComponent->SetupAttachment(RootComponent);
	ItemCollisionComponent->SetBoxExtent(FVector(50.f));
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	ItemCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnItemBeginOverlap);
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_VALID(MapComponent) == false)  // this component is not valid for owner construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponent->OnMapComponentConstruction();

	// Rand the item type
	if (ItemType == EItemTypeEnum::None)
	{
		TArray<EItemTypeEnum> ItemTypesArray;
		ItemTypesByMeshes.GetKeys(ItemTypesArray);
		const int32 RandItemTypeNo = FMath::RandRange(int32(0), ItemTypesArray.Num() - 1);
		ItemType = ItemTypesArray[RandItemTypeNo];
	}
	UStaticMesh* FoundMesh = *ItemTypesByMeshes.Find(ItemType);
	ensureMsgf(FoundMesh != nullptr, TEXT("The item mesh of this type was not found"));
	ItemMesh->SetStaticMesh(FoundMesh);
}

void AItem::OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* const OverlappedCharacter = Cast<AMyCharacter>(OtherActor);
	if (OverlappedCharacter == nullptr				// Other actor is not myCharacter
		|| IS_VALID(OverlappedCharacter) == false)  // Character is not valid
	{
		return;
	}

	switch (ItemType)
	{
		case EItemTypeEnum::Skate:
			OverlappedCharacter->Powerups_.SkateN++;
		case EItemTypeEnum::Bomb:
			OverlappedCharacter->Powerups_.BombN++;
		case EItemTypeEnum::Fire:
			OverlappedCharacter->Powerups_.FireN++;
		default:
			break;
	}

	// Destroy itself on overlapping
	this->Destroy();
}
