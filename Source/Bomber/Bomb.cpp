// Fill out your copyright notice in the Description page of Project Settings.

#include "Bomb.h"

#include "Bomber.h"
#include "Components/StaticMeshComponent.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "MyCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ABomb::ABomb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize root components
	USceneComponent* sceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	sceneComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(sceneComponent);

	// Initializeze bomb mesh
	bombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	bombMesh->SetupAttachment(sceneComponent);
	bombMesh->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));

	// Initialize map component
	mapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

	// Initialize explosion particle component
	ConstructorHelpers::FObjectFinder<UParticleSystem> particleFinder(TEXT("/Game/VFX_Toolkit_V1/ParticleSystems/356Days/Par_CrescentBoom2_OLD"));
	explosionParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("Explosion Particle"));
	if (particleFinder.Succeeded())
	{
		explosionParticle = particleFinder.Object;
	}

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
	if (USingletonLibrary::GetLevelMap(GetWorld()) == nullptr  // levelMap is null
		|| ISVALID(mapComponent) == false)					   // Map component is not valid
	{
		return;
	}

	characterBombN_ = outBombN;

	// Set material
	if (ISVALID(bombMesh) == true)
	{
		const int32 BOMB_MATERIAL_NO = FMath::Abs(characterID) % bombMaterials_.Num();
		bombMesh->SetMaterial(0, bombMaterials_[BOMB_MATERIAL_NO]);
	}

	// Update explosion information
	explosionCells_ = USingletonLibrary::GetLevelMap(GetWorld())->GetSidesCells(mapComponent->cell, fireN, EPathTypesEnum::Explosion);
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

	if (USingletonLibrary::GetLevelMap(GetWorld()) == nullptr  // levelMap is null
		|| ISVALID(mapComponent) == false)					   //map component is not valid
	{
		return;
	}

	if (IsChildActor() == false)  // Was dragged to PIE and it needs to update
	{
		mapComponent->UpdateSelfOnMap();
	}

// Updating own explosions for nongenerated gragged bombs in PIE
#if WITH_EDITOR
	if (GetWorld()->HasBegunPlay() == false)  // for editor only
	{
		explosionCells_ = USingletonLibrary::GetLevelMap(GetWorld())->GetSidesCells(mapComponent->cell, 1, EPathTypesEnum::Explosion);
		UE_LOG_STR("PIE: %s updated own explosions", this);
	}
#endif
}

void ABomb::Destroyed()
{
	if (USingletonLibrary::GetLevelMap(GetWorld()) == nullptr  // levelMap is null
		|| ISVALID(mapComponent) == false)					   //map component is not valid
	{
		return;
	}

	if (characterBombN_ != nullptr)
	{
		(*characterBombN_)++;  // Return to the character +1 of bombs
	}

	// Spawn emitters
	for (const FCell& cell : explosionCells_)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), explosionParticle, FTransform(cell.location));
	}

	// Destroy all actors from array of cells
	USingletonLibrary::GetLevelMap(GetWorld())->DestroyActorsFromMap(explosionCells_);

	Super::Destroyed();
}

void ABomb::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
}
