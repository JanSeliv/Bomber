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
	PrimaryActorTick.bCanEverTick = false;

	// Initialize root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize map component
	MapComponent = CreateDefaultSubobject<UMapComponent>(TEXT("Map Component"));

	// Initialize bomb mesh
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	BombMesh->SetupAttachment(RootComponent);
	BombMesh->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));

	// Initialize explosion particle component
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleFinder(TEXT("/Game/VFX_Toolkit_V1/ParticleSystems/356Days/Par_CrescentBoom2_OLD"));
	ExplosionParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("Explosion Particle"));
	if (ParticleFinder.Succeeded())
	{
		ExplosionParticle = ParticleFinder.Object;
	}

	// Find materials
	const TArray<TCHAR*> Paths{
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Yellow"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Blue"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Silver"),
		TEXT("/Game/Bomber/Assets/MI_Bombs/MI_Bomb_Pink")};
	for (const auto& Path : Paths)
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(Path);
		if (MaterialFinder.Succeeded() == true)
		{
			BombMaterials_.Add(MaterialFinder.Object);
		}
	}
}

void ABomb::InitializeBombProperties(
	int32* OutBombN, const int32& FireN, const int32& CharacterID)
{
	if (USingletonLibrary::GetLevelMap(GetWorld()) == nullptr  // levelMap is null
		|| IS_VALID(MapComponent) == false)					   // Map component is not valid
	{
		return;
	}

	CharacterBombN_ = OutBombN;

	// Set material
	if (IS_VALID(BombMesh) == true)
	{
		const int32 Bomb_Material_No = FMath::Abs(CharacterID) % BombMaterials_.Num();
		BombMesh->SetMaterial(0, BombMaterials_[Bomb_Material_No]);
	}

	// Update explosion information
	ExplosionCells_ = USingletonLibrary::GetLevelMap(GetWorld())->GetSidesCells(MapComponent->Cell, FireN, EPathTypesEnum::Explosion);
}

// Called when the game starts or when spawned
void ABomb::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABomb::OnBombDestroyed);

	SetLifeSpan(LifeSpan_);
}

void ABomb::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_VALID(MapComponent) == false)
	{
		return;
	}

	RootComponent->SetMobility(EComponentMobility::Movable);

	// Update this actor
	MapComponent->UpdateSelfOnMap();

#if WITH_EDITOR
	if (HasActorBegunPlay() == false)  // for editor only
	{
		// Updating own explosions for non generated dragged bombs in PIE
		if (USingletonLibrary::GetLevelMap(GetWorld()) != nullptr)  // levelMap is null
		{
			InitializeBombProperties(nullptr, ExplosionLength, 0);
			UE_LOG_STR("PIE: %s updated own explosions", this);
		}
	}
#endif  //WITH_EDITOR
}

void ABomb::OnBombDestroyed(AActor* DestroyedActor)
{
	UWorld* const World = GetWorld();
	if (World == nullptr									  // World is null
		&& USingletonLibrary::GetLevelMap(World) == nullptr)  // levelMap is null
	{
		return;
	}

	if (CharacterBombN_ != nullptr)
	{
		(*CharacterBombN_)++;  // Return to the character +1 of bombs
	}

	// Spawn emitters
	for (const FCell& Cell : ExplosionCells_)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionParticle, FTransform(Cell.Location));
	}

	// Destroy all actors from array of cells
	USingletonLibrary::GetLevelMap(World)->DestroyActorsFromMap(ExplosionCells_);
}

void ABomb::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
}
