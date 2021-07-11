// Copyright 2021 Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
//---
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"

const ABombActor::FOnBombDestroyed ABombActor::EmptyOnDestroyed = FOnBombDestroyed();

// Default constructor
UBombDataAsset::UBombDataAsset()
{
	ActorTypeInternal = EAT::Bomb;
}

// Returns the bomb data asset
const UBombDataAsset& UBombDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = USingletonLibrary::GetDataAssetByActorType(EActorType::Bomb);
	const auto BombDataAsset = Cast<UBombDataAsset>(FoundDataAsset);
	checkf(BombDataAsset, TEXT("The Bomb Data Asset is not valid"));
	return *BombDataAsset;
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
}

// Sets the defaults of the bomb
void ABombActor::InitBomb(
	const FOnBombDestroyed& EventToBind,
	int32 InFireRadius/* = 1*/,
	int32 CharacterID/* = -1*/)
{
	if (!MapComponentInternal
	    || InFireRadius < 0) // Negative length of the explosion
	{
		return;
	}

	// Set material
	const TArray<TObjectPtr<UMaterialInterface>>& BombMaterials = UBombDataAsset::Get().BombMaterials;
	if (CharacterID != INDEX_NONE // Is not debug character
	    && BombMaterials.Num())   // As least one bomb material
	{
		const int32 BombMaterialNo = FMath::Abs(CharacterID) % BombMaterials.Num();
		MapComponentInternal->SetMaterial(BombMaterials[BombMaterialNo]);
	}

	// Update explosion information
	ExplosionCellsInternal.Empty();
	FireRadiusInternal = InFireRadius;
	AGeneratedMap::Get().GetSidesCells(ExplosionCellsInternal, MapComponentInternal->Cell, EPathType::Explosion, InFireRadius);

	// Notify player that bomb was detonated
	if (EventToBind.IsBound())
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

	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	// Construct the actor's map component
	const bool bIsConstructed = MapComponentInternal->OnConstruction();
	if (!bIsConstructed)
	{
		return;
	}

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		InitBomb(EmptyOnDestroyed, FireRadiusInternal);

		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

		if (MapComponentInternal->bShouldShowRenders)
		{
			USingletonLibrary::ClearOwnerTextRenders(this);

			// Show all cells as yellow and red cells where will be optimized explosions
			FCells EmitterCells;
			AGeneratedMap::Get().GetDangerousCells(EmitterCells, this);
			USingletonLibrary::Get().AddDebugTextRenders(this, EmitterCells, FLinearColor::Red, 266.f);
			USingletonLibrary::Get().AddDebugTextRenders(this, ExplosionCellsInternal, FLinearColor::Yellow);
		}
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ThisClass::DetonateBomb);

	// Binding to the event, that triggered when character end to overlaps the ItemCollisionComponent component
	OnActorEndOverlap.AddDynamic(this, &ABombActor::OnBombEndOverlap);

	// Destroy itself after N seconds
	if (AMyGameStateBase::GetCurrentGameState(this) == ECurrentGameState::InGame)
	{
		SetLifeSpan();
	}
	else if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		// This dragged bomb should start listening in-game state to set lifespan
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Set the lifespan of this actor. When it expires the object will be destroyed
void ABombActor::SetLifeSpan(float InLifespan/* = INDEX_NONE*/)
{
	if (MapComponentInternal
	    && InLifespan == INDEX_NONE) // is default value, should override it
	{
		InLifespan = AGeneratedMap::Get().GetCellLifeSpan(MapComponentInternal->Cell, this);
	}

	Super::SetLifeSpan(InLifespan);
}

// Called when the lifespan of an actor expires (if he has one)
void ABombActor::LifeSpanExpired()
{
	// Override to prevent destroying, do not call super
	DetonateBomb();
}

// Destroy bomb and burst explosion cells
void ABombActor::DetonateBomb(AActor* DestroyedActor/* = nullptr*/)
{
	if (!ExplosionCellsInternal.Num()                                                // no cells to destroy
	    || !IsValid(MapComponentInternal)                                            // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState(this) != ECurrentGameState::InGame) // game was not started or already finished
	{
		return;
	}

	AGeneratedMap& LevelMap = AGeneratedMap::Get();

	// Get only unique cells to spawn emitters once
	FCells UniqueBlastCells;
	LevelMap.GetDangerousCells(UniqueBlastCells, this);

	// Spawn emitters
	UWorld* World = GetWorld();
	UParticleSystem* ExplosionParticle = UBombDataAsset::Get().ExplosionParticle;
	FTransform Position(GetActorTransform());
	for (const FCell& Cell : UniqueBlastCells)
	{
		Position.SetLocation(Cell.Location);
		UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionParticle, Position);
	}

	// Move explosion cells to avoid self destroying one more time by another bomb
	const FCells CellsToDestroy = MoveTemp(ExplosionCellsInternal);

	// Destroy all actors from array of cells
	LevelMap.DestroyActorsFromMap(UniqueBlastCells);
}

// Triggers when character end to overlaps with this bomb.
void ABombActor::OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	UBoxComponent* BombCollisionComponent = MapComponentInternal ? MapComponentInternal->BoxCollision : nullptr;
	if (!BombCollisionComponent // Is not valid collision component
	    || OtherActor == this)  // Self triggering
	{
		return;
	}

	//Sets the collision preset to block all dynamics
	TArray<AActor*> OverlappingActors;
	BombCollisionComponent->GetOverlappingActors(OverlappingActors, USingletonLibrary::GetActorClassByType(EAT::Player));
	if (OverlappingActors.Num() == 0) // There are no more characters on the bomb
	{
		BombCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	}
}

// Listen by dragged bombs to handle game resetting
void ABombActor::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::InGame)
	{
		// Reinit bomb and restart lifespan
		InitBomb(EmptyOnDestroyed, FireRadiusInternal);
		SetLifeSpan(UBombDataAsset::Get().GetLifeSpan());
	}
}
