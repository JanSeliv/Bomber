// Copyright 2020 Yevhenii Selivanov.

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
}

void ABombActor::InitBomb(
	const FOnBombDestroyed& EventToBind,
	int32 FireN/* = 1*/,
	int32 CharacterID/* = -1*/)
{
	if (!IsValid(USingletonLibrary::GetLevelMap()) // // The Level Map is not valid
	    || !IsValid(MapComponentInternal)          // The Map Component is not valid
	    || FireN < 0)                              // Negative length of the explosion
	{
		return;
	}

	// Set material
	const TArray<UMaterialInterface*>& BombMaterials = MapComponentInternal->GetDataAssetChecked<UBombDataAsset>()->BombMaterials;
	if (CharacterID != -1       // Is not debug character
	    && BombMaterials.Num()) // As least one bomb material
	{
		const int32 BombMaterialNo = FMath::Abs(CharacterID) % BombMaterials.Num();
		MapComponentInternal->SetMaterial(BombMaterials[BombMaterialNo]);
	}

	// Update explosion information
	ExplosionCellsInternal.Empty();
	USingletonLibrary::GetLevelMap()->GetSidesCells(ExplosionCellsInternal, MapComponentInternal->Cell, EPathType::Explosion, FireN);

#if WITH_EDITOR  // [Editor]
	if (MapComponentInternal->bShouldShowRenders)
	{
		USingletonLibrary::PrintToLog(this, "[Editor]InitializeBombProperties", "-> \t AddDebugTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(this);
		bool bOutBool = false;
		TArray<UTextRenderComponent*> OutArray;
		USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, ExplosionCellsInternal, FLinearColor::Red, bOutBool, OutArray);
	}
#endif

	// Notify player that bomb was detonated
	OnDestroyed.Add(EventToBind);
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
	MapComponentInternal->OnConstruction();

#if WITH_EDITOR
	if (USingletonLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
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
	OnDestroyed.AddDynamic(this, &ThisClass::DetonateBomb);

	// Binding to the event, that triggered when character end to overlaps the ItemCollisionComponent component
	OnActorEndOverlap.AddDynamic(this, &ABombActor::OnBombEndOverlap);

	// Destroy itself after N seconds
	if (AMyGameStateBase::GetCurrentGameState(this) == ECurrentGameState::InGame)
	{
		SetLifeSpan();
	}
	else if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState(this))
	{
		// This dragged bomb should start listening in-game state to set lifespan
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Set the lifespan of this actor. When it expires the object will be destroyed
void ABombActor::SetLifeSpan(float InLifespan/* = INDEX_NONE*/)
{
	if (InLifespan == INDEX_NONE) // is default value, should override it
	{
		const UBombDataAsset* BombDataAsset = MapComponentInternal ? MapComponentInternal->GetDataAssetChecked<UBombDataAsset>() : nullptr;
		if (BombDataAsset)
		{
			InLifespan = BombDataAsset->GetLifeSpan();
		}
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
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (!LevelMap                                                                    // The Level Map is not valid or transient (in regenerating process)
	    || !ExplosionCellsInternal.Num()                                             // no cells to destroy
	    || !IsValid(MapComponentInternal)                                            // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState(this) != ECurrentGameState::InGame) // game was not started or already finished
	{
		return;
	}

	// Spawn emitters
	UParticleSystem* ExplosionParticle = MapComponentInternal->GetDataAssetChecked<UBombDataAsset>()->ExplosionParticle;
	for (const FCell& Cell : ExplosionCellsInternal)
	{
		const FTransform Position{GetActorRotation(), Cell.Location, GetActorScale3D()};
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, Position);
	}

	// Copy explosion cells and empty it to avoid self destroying one more time by another bomb
	const FCells CellsToDestroy{ExplosionCellsInternal};
	ExplosionCellsInternal.Empty();

	// Destroy all actors from array of cells
	LevelMap->DestroyActorsFromMap(CellsToDestroy);
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
		InitBomb(FOnBombDestroyed());
		SetLifeSpan();
	}
}
