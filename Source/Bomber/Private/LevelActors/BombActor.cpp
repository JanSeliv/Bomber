// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "DataAssets/BombDataAsset.h"
#include "DataAssets/DataAssetsContainer.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Structures/Cell.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(BombActor)

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

// Initialize a bomb actor, could be called multiple times
void ABombActor::ConstructBombActor()
{
	checkf(MapComponentInternal, TEXT("%s: 'MapComponentInternal' is null"), *FString(__FUNCTION__));
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionBombActor);
	MapComponentInternal->ConstructOwnerActor();
}

// Returns cells that bombs is going to destroy
FCells ABombActor::GetExplosionCells() const
{
	if (IsHidden()
	    || FireRadiusInternal < MIN_FIRE_RADIUS
	    || !MapComponentInternal)
	{
		return FCell::EmptyCells;
	}

	return UCellsUtilsLibrary::GetCellsAround(MapComponentInternal->GetCell(), EPathType::Explosion, FireRadiusInternal);
}

// Sets the defaults of the bomb
void ABombActor::InitBomb(const APlayerCharacter* Causer/* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	int32 InFireRadius = MIN_FIRE_RADIUS;
	int32 CharacterID = INDEX_NONE;
	ELevelType PlayerType = ELevelType::None;
	if (Causer)
	{
		CharacterID = Causer->GetCharacterID();
		InFireRadius = Causer->GetPowerups().FireN;
		PlayerType = Causer->GetPlayerType();
	}

	const UBombDataAsset& BombDataAsset = UBombDataAsset::Get();

	const ULevelActorRow* BombRow = BombDataAsset.GetRowByLevelType(PlayerType);
	const UStaticMesh* BombMesh = BombRow ? Cast<UStaticMesh>(BombRow->Mesh) : nullptr;
	if (ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %s:\n'BombMesh' is not found"), __LINE__, *FString(__FUNCTION__)))
	{
		// Override mesh and material
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %s:\n'MapComponentInternal' is null!"), __LINE__, *FString(__FUNCTION__));
		MapComponentInternal->SetLevelActorRow(BombRow);
		BombMaterialInternal = BombMesh->GetMaterial(0);
	}

	MapComponentInternal->SetLevelActorRow(BombRow);
	if (PlayerType == ELevelType::None)
	{
		// Is bot character, set material for its default bomb with the same mesh
		const int32 BombMaterialsNum = BombDataAsset.GetBombMaterialsNum();
		if (CharacterID != INDEX_NONE // Is not debug character
		    && BombMaterialsNum)      // As least one bomb material
		{
			const int32 MaterialIndex = FMath::Abs(CharacterID) % BombMaterialsNum;
			BombMaterialInternal = BombDataAsset.GetBombMaterial(MaterialIndex);
		}
	}

	ApplyMaterial();

	FireRadiusInternal = InFireRadius;

	UpdateCollisionResponseToAllPlayers();

	TryDisplayExplosionCells();
}

// Show current explosion cells if the bomb type is allowed to be displayed, is not available in shipping build
void ABombActor::TryDisplayExplosionCells()
{
#if !UE_BUILD_SHIPPING
	if (UCellsUtilsLibrary::CanDisplayCellsForActorTypes(TO_FLAG(EAT::Bomb)))
	{
		FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
		Params.TextColor = FLinearColor::Red;
		Params.TextHeight += 1.f;
		UCellsUtilsLibrary::DisplayCells(this, GetExplosionCells(), Params);
	}
#endif // !UE_BUILD_SHIPPING
}

/* ---------------------------------------------------
 *					Protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned.
void ABombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructBombActor();
}

// Is called on a bomb actor construction, could be called multiple times
void ABombActor::OnConstructionBombActor()
{
	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		InitBomb();

		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();

		if (MapComponentInternal->bShouldShowRenders)
		{
			FDisplayCellsParams DisplayParams;
			DisplayParams.TextColor = FLinearColor::Red;
			DisplayParams.bClearPreviousDisplays = true;
			UCellsUtilsLibrary::DisplayCells(this, GetExplosionCells(), DisplayParams);
		}
	}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	// Destroy itself after N seconds
	if (AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::InGame)
	{
		SetLifeSpan();
	}
	else if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		// This dragged bomb should start listening in-game state to set lifespan
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);

		// Handle current game state if initialized with delay
		if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
		{
			OnGameStateChanged(ECurrentGameState::Menu);
		}
	}
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, FireRadiusInternal);
	DOREPLIFETIME(ThisClass, BombMaterialInternal);
}

// Set the lifespan of this actor. When it expires the object will be destroyed
void ABombActor::SetLifeSpan(float InLifespan/* = DEFAULT_LIFESPAN*/)
{
	if (InLifespan == DEFAULT_LIFESPAN
	    && AMyGameStateBase::GetCurrentGameState() == ECGS::InGame)
	{
		InLifespan = UBombDataAsset::Get().GetLifeSpan();
	}

	Super::SetLifeSpan(InLifespan);
}

// Called when the lifespan of an actor expires (if he has one)
void ABombActor::LifeSpanExpired()
{
	// Override to prevent destroying, do not call super

	DetonateBomb();
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void ABombActor::SetActorHiddenInGame(bool bNewHidden)
{
	if (HasAuthority())
	{
		if (!bNewHidden)
		{
			// Is added on Generated Map

			ConstructBombActor();

			SetLifeSpan();

			// Binding to the event, that triggered when character end to overlaps the collision component
			OnActorEndOverlap.AddUniqueDynamic(this, &ABombActor::OnBombEndOverlap);
		}
		else
		{
			// Bomb is removed from Generated Map, detonate it
			DetonateBomb();

			OnActorEndOverlap.RemoveDynamic(this, &ABombActor::OnBombEndOverlap);
		}
	}

	// Apply hidden flag
	Super::SetActorHiddenInGame(bNewHidden);
}

void ABombActor::DetonateBomb()
{
	if (!HasAuthority()
	    || IsHidden()
	    || FireRadiusInternal < MIN_FIRE_RADIUS
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	MulticastDetonateBomb();
}

// Destroy bomb and burst explosion cells
void ABombActor::MulticastDetonateBomb_Implementation()
{
	const FCells ExplosionCells = GetExplosionCells();
	if (ExplosionCells.IsEmpty())
	{
		// No cells to destroy
		return;
	}

	// Reset Fire Radius to avoid destroying the bomb again
	FireRadiusInternal = INDEX_NONE;

	// Spawn emitters
	UNiagaraSystem* ExplosionParticle = UBombDataAsset::Get().GetExplosionVFX();
	for (const FCell& Cell : ExplosionCells)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionParticle, Cell.Location, GetActorRotation(), GetActorScale());
	}

	// Destroy all actors from array of cells
	AGeneratedMap::Get().DestroyLevelActorsOnCells(ExplosionCells, this);

	USoundsSubsystem::Get().PlayExplosionSFX();

	GetWorldTimerManager().ClearTimer(TimerHandle_LifeSpanExpired);
}

// Triggers when character end to overlaps with this bomb.
void ABombActor::OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	const bool bIsPlayerOverlap = Cast<APlayerCharacter>(OtherActor) != nullptr;
	if (!bIsPlayerOverlap)
	{
		return;
	}

	UpdateCollisionResponseToAllPlayers();
}

// Listen by dragged bombs to handle game resetting
void ABombActor::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::InGame)
	{
		// Reinit bomb and restart lifespan
		InitBomb();
		SetLifeSpan();
	}
}

// Sets actual collision responses to all players for this bomb
void ABombActor::UpdateCollisionResponseToAllPlayers()
{
	checkf(MapComponentInternal, TEXT("%s: 'MapComponentInternal' is null"), *FString(__FUNCTION__));
	FCollisionResponseContainer CollisionResponses = MapComponentInternal->GetCollisionResponses();

	TArray<AActor*> OverlappingPlayers;
	GetOverlappingPlayers(OverlappingPlayers);
	if (!OverlappingPlayers.Num())
	{
		// There are no characters on the bomb, block all
		MakeCollisionResponseToAllPlayers(/*out*/CollisionResponses, ECR_Block);
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
		MakeCollisionResponseToPlayersInBitmask(/*out*/CollisionResponses, Bitmask, BitOnResponse, BitOffResponse);
	}

	MapComponentInternal->SetCollisionResponses(CollisionResponses);
}

// Takes your container and returns is with new specified response for player by its specified ID
void ABombActor::MakeCollisionResponseToPlayerByID(FCollisionResponseContainer& InOutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse)
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

	InOutCollisionResponses.SetResponse(CollisionChannel, NewResponse);
}

// Takes your container and returns new specified response for all players
void ABombActor::MakeCollisionResponseToAllPlayers(FCollisionResponseContainer& InOutCollisionResponses, ECollisionResponse NewResponse)
{
	static constexpr int32 BitsOffOnly = 0;
	constexpr ECollisionResponse BitOnResponse = ECR_MAX;
	MakeCollisionResponseToPlayersInBitmask(InOutCollisionResponses, BitsOffOnly, BitOnResponse, NewResponse);
}

// Takes your container and returns new specified response for those players who match their ID in specified bitmask
void ABombActor::MakeCollisionResponseToPlayersInBitmask(FCollisionResponseContainer& InOutCollisionResponses, int32 Bitmask, ECollisionResponse BitOnResponse, ECollisionResponse BitOffResponse)
{
	static constexpr int32 MaxPlayerID = 3;
	for (int32 CharacterID = 0; CharacterID <= MaxPlayerID; ++CharacterID)
	{
		const bool bIsBitOn = ((1 << CharacterID) & Bitmask) != 0;
		const ECollisionResponse Response = bIsBitOn ? BitOnResponse : BitOffResponse;
		MakeCollisionResponseToPlayerByID(InOutCollisionResponses, CharacterID, Response);
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
