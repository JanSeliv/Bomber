// Fill out your copyright notice in the Description page of Project Settings.

#include "Bomb.h"

#include "Bomber.h"
#include "Components/StaticMeshComponent.h"
#include "GeneratedMap.h"
#include "MyCharacter.h"

// Sets default values
ABomb::ABomb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize components
	mapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));
	bombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	SetRootComponent(bombMesh);

	// Find materials
	const TArray<TCHAR*> pathes{
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Yellow"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Blue"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Silver"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Pink")};
	for (const auto& path : pathes)
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> materialFinder(path);
		if (materialFinder.Succeeded() == true)
		{
			bombMaterials_.Add(materialFinder.Object);
		}
	}
}

void ABomb::InitializeBombProperties(
	int32* outBombN, const int32& fireN, const int32& characterID)
{
	characterBombN_ = outBombN;

	// Set material
	const int32 BOMB_MATERIAL_NO = characterID % bombMaterials_.Num();
	bombMesh->SetMaterial(0, bombMaterials_[BOMB_MATERIAL_NO]);

	// Update explosion information
	explosionCells_ = USingletonLibrary::GetLevelMap()->GetSidesCells(mapComponent->cell, fireN, EPathTypesEnum::Explosion);
}

// Called when the game starts or when spawned
void ABomb::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(lifeSpan_);
}

void ABomb::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	AGeneratedMap* levelMap = USingletonLibrary::GetLevelMap();

	if (ISVALID(mapComponent) == false  // map component was not initialized
		|| ISVALID(levelMap) == false)  // level map is not valid
	{
		return;
	}

	mapComponent->UpdateSelfOnMap();

	// Updating explosions for nongenerated gragged bombs in PIE
	if (HasActorBegunPlay() == false)
	{
		explosionCells_ = USingletonLibrary::GetLevelMap()->GetSidesCells(mapComponent->cell, 1, EPathTypesEnum::Explosion);
	}
}

void ABomb::Destroyed()
{
	if (characterBombN_ != nullptr)
	{
		(*characterBombN_)++;
	}
	Super::Destroyed();
}

void ABomb::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
}
