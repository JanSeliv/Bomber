// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "SoundsManager.h"
#include "PoolManager.h"
//---
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#endif

// Default constructor
UBombDataAsset::UBombDataAsset()
{
	ActorTypeInternal = EAT::Bomb;
}

// Returns the bomb data asset
const UBombDataAsset& UBombDataAsset::Get()
{
	const ULevelActorDataAsset* FoundDataAsset = UDataAssetsContainer::GetDataAssetByActorType(EActorType::Bomb);
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
	const int32 BombMaterialsNum = UBombDataAsset::Get().GetBombMaterialsNum();
	if (CharacterID != INDEX_NONE // Is not debug character
	    && BombMaterialsNum)      // As least one bomb material
	{
		const int32 MaterialIndex = FMath::Abs(CharacterID) % BombMaterialsNum;
		BombMaterialInternal = UBombDataAsset::Get().GetBombMaterial(MaterialIndex);
		ApplyMaterial();
	}

	// Update explosion information

	FireRadiusInternal = InFireRadius;
	const FCells ExplosionCells = UCellsUtilsLibrary::GetCellsAround(MapComponentInternal->GetCell(), EPathType::Explosion, InFireRadius);
	ExplosionCellsInternal = ExplosionCells.Array();

	FCollisionResponseContainer CollisionResponses = MapComponentInternal->GetCollisionResponses();

	TArray<AActor*> OverlappingPlayers;
	GetOverlappingPlayers(OverlappingPlayers);
	if (!OverlappingPlayers.Num())
	{
		// There are no characters on the bomb, block all
		GetCollisionResponseToAllPlayers(/*out*/CollisionResponses, ECR_Block);
		OnActorEndOverlap.RemoveDynamic(this, &ABombActor::OnBombEndOverlap);
	}
	else
	{
		// Add to bitmask overlapping players
		int32 Bitmask = 0;
		for (const AActor* OverlappingPlayerIt : OverlappingPlayers)
		{
			if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlappingPlayerIt))
			{
				Bitmask |= 1 << PlayerCharacter->GetCharacterID();
			}
		}

		// Set overlap response for overlapping players, block others
		CollisionResponses = MapComponentInternal->GetCollisionResponses();
		constexpr ECollisionResponse BitOnResponse = ECR_Overlap;
		constexpr ECollisionResponse BitOffResponse = ECR_Block;
		GetCollisionResponseToPlayers(/*out*/CollisionResponses, Bitmask, BitOnResponse, BitOffResponse);
	}

	MapComponentInternal->SetCollisionResponses(CollisionResponses);
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
	if (UEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		InitBomb();

		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

		if (MapComponentInternal->bShouldShowRenders)
		{
			USingletonLibrary::ClearDisplayedCells(this);
			static const FDisplayCellsParams DisplayParams{FLinearColor::Red};
			USingletonLibrary::DisplayCells(this, FCells(ExplosionCellsInternal), DisplayParams);
		}
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Binding to the event, that triggered when the actor has been explicitly destroyed
		OnDestroyed.AddDynamic(this, &ThisClass::MulticastDetonateBomb);
	}

	// Destroy itself after N seconds
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::InGame)
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
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
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

		// Binding to the event, that triggered when character end to overlaps the collision component
		OnActorEndOverlap.AddUniqueDynamic(this, &ABombActor::OnBombEndOverlap);

		return;
	}

	// Bomb is removed from level map, detonate it
	MulticastDetonateBomb();
	FireRadiusInternal = 1;

	OnActorEndOverlap.RemoveDynamic(this, &ABombActor::OnBombEndOverlap);
}

// Destroy bomb and burst explosion cells
void ABombActor::MulticastDetonateBomb_Implementation(AActor* DestroyedActor/* = nullptr*/)
{
	if (!ExplosionCellsInternal.Num()                               // no cells to destroy
	    || !IsValid(MapComponentInternal)                           // The Map Component is not valid or is destroyed already
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame) // game was not started or already finished
	{
		return;
	}

	// Spawn emitters
	UNiagaraSystem* ExplosionParticle = UBombDataAsset::Get().GetExplosionVFX();
	for (const FCell& Cell : ExplosionCellsInternal)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionParticle, Cell.Location, GetActorRotation(), GetActorScale());
	}

	// Move explosion cells to avoid self destroying one more time by another bomb
	const FCells CellsToDestroy(ExplosionCellsInternal);
	ExplosionCellsInternal.Empty();

	// Destroy all actors from array of cells
	AGeneratedMap::Get().DestroyActorsFromMap(CellsToDestroy);

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
	if (!PlayerCharacter
	    || !MapComponentInternal)
	{
		return;
	}

	FCollisionResponseContainer CollisionResponses = MapComponentInternal->GetCollisionResponses();

	TArray<AActor*> OverlappingPlayers;
	GetOverlappingPlayers(OverlappingPlayers);
	if (!OverlappingPlayers.Num())
	{
		// There are no more characters on the bomb
		GetCollisionResponseToAllPlayers(/*out*/CollisionResponses, ECR_Block);
		OnActorEndOverlap.RemoveDynamic(this, &ABombActor::OnBombEndOverlap);
	}
	else
	{
		// Start block only the player that left this bomb
		const int32 CharacterID = PlayerCharacter->GetCharacterID();
		GetCollisionResponseToPlayer(/*out*/CollisionResponses, CharacterID, ECR_Block);
	}

	MapComponentInternal->SetCollisionResponses(CollisionResponses);
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
void ABombActor::GetCollisionResponseToPlayer(FCollisionResponseContainer& OutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse) const
{
	if (CharacterID < 0)
	{
		return;
	}

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

	OutCollisionResponses.SetResponse(CollisionChannel, NewResponse);
}

// Changes the response for all players
void ABombActor::GetCollisionResponseToAllPlayers(FCollisionResponseContainer& OutCollisionResponses, ECollisionResponse NewResponse) const
{
	static constexpr int32 BitsOffOnly = 0;
	constexpr ECollisionResponse BitOnResponse = ECR_MAX;
	GetCollisionResponseToPlayers(OutCollisionResponses, BitsOffOnly, BitOnResponse, NewResponse);
}

// Changes the response for players by specified bitmask
void ABombActor::GetCollisionResponseToPlayers(FCollisionResponseContainer& OutCollisionResponses, int32 Bitmask, ECollisionResponse BitOnResponse, ECollisionResponse BitOffResponse) const
{
	static constexpr int32 MaxPlayerID = 3;
	for (int32 CharacterID = 0; CharacterID <= MaxPlayerID; ++CharacterID)
	{
		const bool bIsBitOn = ((1 << CharacterID) & Bitmask) != 0;
		const ECollisionResponse Response = bIsBitOn ? BitOnResponse : BitOffResponse;
		GetCollisionResponseToPlayer(OutCollisionResponses, CharacterID, Response);
	}
}

// Returns all players overlapping with this bomb
void ABombActor::GetOverlappingPlayers(TArray<AActor*>& OutPlayers) const
{
	const UBoxComponent* BombCollisionComponent = MapComponentInternal ? MapComponentInternal->GetBoxCollisionComponent() : nullptr;
	if (BombCollisionComponent)
	{
		BombCollisionComponent->GetOverlappingActors(OutPlayers, UDataAssetsContainer::GetActorClassByType(EAT::Player));
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
