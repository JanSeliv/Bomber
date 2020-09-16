// Copyright 2020 Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "MapComponent.h"
#include "SingletonLibrary.h"
//---
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Default constructor
UBombDataAsset::UBombDataAsset()
{
	ActorTypeInternal = EAT::Bomb;
}

// Sets default values
ABombActor::ABombActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));

	// Initialize bomb mesh component
	BombMeshComponentInternal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMeshComponent"));
	BombMeshComponentInternal->SetupAttachment(RootComponent);
}

void ABombActor::InitBomb(
	const FOnBombDestroyed& EventToBind,
	const int32& FireN /*= 1*/,
	const int32& CharacterID /*=-1*/)
{
	if (!IsValid(USingletonLibrary::GetLevelMap()) // // The Level Map is not valid
	    || !IsValid(MapComponentInternal)                  // The Map Component is not valid
	    || FireN < 0)                              // Negative length of the explosion
	{
		return;
	}

	// Set material
	const TArray<UMaterialInterface*>& BombMaterials = MapComponentInternal->GetDataAssetChecked<UBombDataAsset>()->BombMaterials;
	if (IsValid(BombMeshComponentInternal)	// Mesh of the bomb is not valid
        && CharacterID != -1		// Is not debug character
        && BombMaterials.Num())		// As least one bomb material
	{
		const int32 BombMaterialNo = FMath::Abs(CharacterID) % BombMaterials.Num();
		BombMeshComponentInternal->SetMaterial(0, BombMaterials[BombMaterialNo]);
	}

	// Update explosion information
	USingletonLibrary::GetLevelMap()->GetSidesCells(ExplosionCellsInternal, MapComponentInternal->Cell, EPathType::Explosion, FireN);

	#if WITH_EDITOR  // [Editor]
		if (MapComponentInternal->bShouldShowRenders)
		{
			USingletonLibrary::PrintToLog(this, "[Editor]InitializeBombProperties", "-> \t AddDebugTextRenders");
			USingletonLibrary::AddDebugTextRenders(this, ExplosionCellsInternal.Array(), FLinearColor::Red);
		}
	#endif

	if(EventToBind.IsBound())
	{
		OnDestroyed.Add(EventToBind);
	}
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned.
void ABombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this)			// This actor is transient
		|| !IsValid(MapComponentInternal))	// Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component
	MapComponentInternal->OnComponentConstruct(BombMeshComponentInternal, FLevelActorMeshRow::Empty);

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld())  // [IsEditorNotPieWorld]
	{
		USingletonLibrary::PrintToLog(this, "[IsEditorNotPieWorld]OnConstruction", "-> \t InitializeBombProperties");
		InitBomb(FOnBombDestroyed());

		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ABombActor::OnBombDestroyed);

	// Binding to the event, that triggered when character end to overlaps the ItemCollisionComponent component
	OnActorEndOverlap.AddDynamic(this, &ABombActor::OnBombEndOverlap);

	// Destroy itself after N seconds
	const UBombDataAsset* BombDataAsset = MapComponentInternal ? Cast<UBombDataAsset>(MapComponentInternal->GetActorDataAsset()) : nullptr;
	if(BombDataAsset)
	{
		SetLifeSpan(BombDataAsset->GetLifeSpan());
	}
}

// Calls destroying request of all actors by cells in explosion cells array.
void ABombActor::OnBombDestroyed(AActor* DestroyedActor)
{
	// Spawn emitters
	if(MapComponentInternal)
	{
		UParticleSystem* ExplosionParticle = MapComponentInternal->GetDataAssetChecked<UBombDataAsset>()->ExplosionParticle;
		for (const FCell& Cell : ExplosionCellsInternal)
		{
			const FTransform Position{GetActorRotation(), Cell.Location, GetActorScale3D()};
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, Position);
        }
	}

	// Destroy all actors from array of cells
	if (AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap())	// The Level Map is not valid
	{
		LevelMap->DestroyActorsFromMap(ExplosionCellsInternal);
	}
}

// Triggers when character end to overlaps with this bomb.
void ABombActor::OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	UBoxComponent* BombCollisionComponent = MapComponentInternal ? MapComponentInternal->BoxCollision : nullptr;
	if (!BombCollisionComponent	// Is not valid collision component
		|| OtherActor == this)	// Self triggering
	{
		return;
	}

	//Sets the collision preset to block all dynamics
	TArray<AActor*> OverlappingActors;
	BombCollisionComponent->GetOverlappingActors(OverlappingActors, USingletonLibrary::GetActorClassByType(EAT::Player));
	if (OverlappingActors.Num() == 0)  // There are no more characters on the bomb
	{
		BombCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	}
}
