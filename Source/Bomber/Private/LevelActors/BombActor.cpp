// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "SoundsManager.h"
#include "PoolManager.h"
//---
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

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

	// Replicate an actor
	bReplicates = true;
	NetUpdateFrequency = 10.f;
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponentInternal = CreateDefaultSubobject<UMapComponent>(TEXT("MapComponent"));
}

// Sets the defaults of the bomb
void ABombActor::InitBomb(int32 InFireRadius/* = DEFAULT_FIRE_RADIUS*/, int32 CharacterID/* = -1*/)
{
	if (!HasAuthority()
	    || !MapComponentInternal
	    || InFireRadius < 0) // Negative length of the explosion
	{
		return;
	}

	// Set material
	const TArray<TObjectPtr<UMaterialInterface>>& BombMaterials = UBombDataAsset::Get().BombMaterials;
	if (CharacterID != INDEX_NONE // Is not debug character
	    && BombMaterials.Num())   // As least one bomb material
	{
		const int32 MaterialIndex = FMath::Abs(CharacterID) % BombMaterials.Num();
		BombMaterialInternal = BombMaterials[MaterialIndex];
		ApplyMaterial();
	}

	// Update explosion information
	FCells ExplosionCells;
	FireRadiusInternal = InFireRadius;
	AGeneratedMap::Get().GetSidesCells(ExplosionCells, MapComponentInternal->GetCell(), EPathType::Explosion, InFireRadius);
	ExplosionCellsInternal = ExplosionCells.Array();

	// Set default collision to block all players
	SetCollisionResponseToAllPlayers(ECR_Block);

	// Do not block overlapping players
	TArray<AActor*> OverlappingPlayers;
	GetOverlappingPlayers(OverlappingPlayers);
	for (const AActor* OverlappingPlayerIt : OverlappingPlayers)
	{
		if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlappingPlayerIt))
		{
			SetCollisionResponseToPlayer(PlayerCharacter->GetCharacterID(), ECR_Overlap);
		}
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
		InitBomb();

		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

		if (MapComponentInternal->bShouldShowRenders)
		{
			USingletonLibrary::ClearOwnerTextRenders(this);
			USingletonLibrary::Get().AddDebugTextRenders(this, FCells(ExplosionCellsInternal), FLinearColor::Red);
		}
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	// Binding to the event, that triggered when the actor has been explicitly destroyed
	OnDestroyed.AddDynamic(this, &ThisClass::MulticastDetonateBomb);

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

// Returns properties that are replicated for the lifetime of the actor channel
void ABombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ExplosionCellsInternal);
	DOREPLIFETIME(ThisClass, FireRadiusInternal);
	DOREPLIFETIME(ThisClass, BombMaterialInternal);
}

// Set the lifespan of this actor. When it expires the object will be destroyed
void ABombActor::SetLifeSpan(float InLifespan/* = INDEX_NONE*/)
{
	if (AMyGameStateBase::GetCurrentGameState(this) != ECGS::InGame)
	{
		// Do not allow trigger life span when match is not started
		return;
	}

	if (MapComponentInternal
	    && InLifespan == INDEX_NONE)
	{
		InLifespan = UBombDataAsset::Get().GetLifeSpan();
	}

	Super::SetLifeSpan(InLifespan);
}

// Called when the lifespan of an actor expires (if he has one)
void ABombActor::LifeSpanExpired()
{
	// Override to prevent destroying, do not call super

	MulticastDetonateBomb();
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void ABombActor::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (!HasAuthority())
	{
		return;
	}

	if (!bNewHidden)
	{
		// Is added on level map
		SetLifeSpan();
		return;
	}

	// Bomb is removed from level map, detonate it
	MulticastDetonateBomb();
	FireRadiusInternal = 1;
}

// Destroy bomb and burst explosion cells
void ABombActor::MulticastDetonateBomb_Implementation(AActor* DestroyedActor/* = nullptr*/)
{
	if (!ExplosionCellsInternal.Num()                                   // no cells to destroy
	    || !IsValid(MapComponentInternal)                               // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState(this) != ECGS::InGame) // game was not started or already finished
	{
		return;
	}

	AGeneratedMap& LevelMap = AGeneratedMap::Get();

	// Spawn emitters
	UNiagaraSystem* ExplosionParticle = UBombDataAsset::Get().ExplosionParticle;
	for (const FCell& Cell : ExplosionCellsInternal)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionParticle, Cell.Location, GetActorRotation(), GetActorScale());
	}

	// Move explosion cells to avoid self destroying one more time by another bomb
	const FCells CellsToDestroy(ExplosionCellsInternal);
	ExplosionCellsInternal.Empty();

	// Destroy all actors from array of cells
	LevelMap.DestroyActorsFromMap(CellsToDestroy);

	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayExplosionSFX();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_LifeSpanExpired);
}

// Triggers when character end to overlaps with this bomb.
void ABombActor::OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	TArray<AActor*> OverlappingPlayers;
	GetOverlappingPlayers(OverlappingPlayers);
	if (!OverlappingPlayers.Num())
	{
		// There are no more characters on the bomb
		SetCollisionResponseToAllPlayers(ECR_Block);
		return;
	}

	// Start block only the player that left this bomb
	SetCollisionResponseToPlayer(PlayerCharacter->GetCharacterID(), ECR_Block);
}

// Listen by dragged bombs to handle game resetting
void ABombActor::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::InGame)
	{
		// Reinit bomb and restart lifespan
		InitBomb();
		SetLifeSpan(UBombDataAsset::Get().GetLifeSpan());
	}
}

// Changes the response for specified player
void ABombActor::SetCollisionResponseToPlayer(int32 CharacterID, ECollisionResponse NewResponse)
{
	if (!ensureMsgf(MapComponentInternal, TEXT("ASSERT: 'MapComponentInternal' is not valid"))
	    || CharacterID < 0)
	{
		return;
	}

	FCollisionResponseContainer CollisionResponses = MapComponentInternal->GetCollisionResponses();

	ECollisionChannel CollisionChannel = ECC_WorldDynamic;
	switch (CharacterID)
	{
		case 0:
			CollisionChannel = ECC_Player0;
			break;
		case 1:
			CollisionChannel = ECC_Player1;
			break;
		case 2:
			CollisionChannel = ECC_Player2;
			break;
		case 3:
			CollisionChannel = ECC_Player3;
			break;
		default:
			break;
	}

	CollisionResponses.SetResponse(CollisionChannel, NewResponse);
	MapComponentInternal->SetCollisionResponses(CollisionResponses);
}

// Changes the response for all players
void ABombActor::SetCollisionResponseToAllPlayers(ECollisionResponse NewResponse)
{
	static constexpr int32 MaxPlayerID = 3;
	for (int32 CharacterID = 0; CharacterID <= MaxPlayerID; ++CharacterID)
	{
		SetCollisionResponseToPlayer(CharacterID, NewResponse);
	}
}

// Returns all players overlapping with this bomb
void ABombActor::GetOverlappingPlayers(TArray<AActor*>& OutPlayers) const
{
	const UBoxComponent* BombCollisionComponent = MapComponentInternal ? MapComponentInternal->GetBoxCollisionComponent() : nullptr;
	if (BombCollisionComponent)
	{
		BombCollisionComponent->GetOverlappingActors(OutPlayers, USingletonLibrary::GetActorClassByType(EAT::Player));
	}
}

// Updates current material for this bomb actor
void ABombActor::ApplyMaterial()
{
	if (!BombMaterialInternal
	    || !MapComponentInternal)
	{
		return;
	}

	MapComponentInternal->SetMaterial(BombMaterialInternal);
}

// Is called on client to respond on changes in material of the bomb
void ABombActor::OnRep_BombMaterial()
{
	ApplyMaterial();
}
