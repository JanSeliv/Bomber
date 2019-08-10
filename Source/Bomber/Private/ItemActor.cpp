// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"

#include "Bomber.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MapComponent.h"
#include "MyCharacter.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AItemActor::AItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize item mesh component
	ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMeshComponent->SetupAttachment(RootComponent);

	// Find and fill item meshes array
	static TArray<ConstructorHelpers::FObjectFinder<UStaticMesh>> ItemMeshFinderArray{
		TEXT("/Game/Bomber/Assets/Meshes/SkateItemMesh"),
		TEXT("/Game/Bomber/Assets/Meshes/BombItemMesh"),
		TEXT("/Game/Bomber/Assets/Meshes/FireItemMesh")};
	for (int32 i = 0; i < ItemMeshFinderArray.Num(); ++i)
	{
		if (ItemMeshFinderArray[i].Succeeded())
		{
			ItemTypesByMeshes.Add(static_cast<EItemTypeEnum>(i + 1), ItemMeshFinderArray[i].Object);
		}
	}

	// Initialize the Item Collision Component to allow players moving through this item to pick up it
	UBoxComponent* ItemCollisionComponent = CreateDefaultSubobject<UBoxComponent>("ItemCollisionComponent");
	ItemCollisionComponent->SetupAttachment(RootComponent);
	ItemCollisionComponent->SetBoxExtent(FVector(50.f));
	ItemCollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	this->OnActorBeginOverlap.AddDynamic(this, &AItemActor::OnItemBeginOverlap);
}

void AItemActor::OnConstruction(const FTransform& Transform)
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
	ItemMeshComponent->SetStaticMesh(FoundMesh);
}

void AItemActor::OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
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
			break;
		case EItemTypeEnum::Bomb:
			OverlappedCharacter->Powerups_.BombN++;
			break;
		case EItemTypeEnum::Fire:
			OverlappedCharacter->Powerups_.FireN++;
			break;
		default:
			ensureMsgf(ItemType != EItemTypeEnum::None, TEXT("None type of the item"));
			break;
	}

	// Destroy itself on overlapping
	this->Destroy();
}
