// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BombActor.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Components/MapComponent.h"
#include "DataAssets/BombDataAsset.h"
#include "DataAssets/DataAssetsContainer.h"
#include "GameFramework/MyCheatManager.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "Structures/Cell.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
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
	static constexpr float NewNewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewNewUpdateFrequency);
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

//  Returns the type of the bomb
ELevelType ABombActor::GetBombType() const
{
	return MapComponentInternal ? MapComponentInternal->GetLevelType() : ELevelType::None;
}

// Applies the bomb type. It impacts the bomb mesh, material and VFX
void ABombActor::SetBombType(ELevelType InBombType)
{
	const ULevelActorRow* BombRow = UBombDataAsset::Get().GetRowByLevelType(InBombType);
	UStaticMesh* BombMesh = BombRow ? Cast<UStaticMesh>(BombRow->Mesh) : nullptr;
	if (!ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %hs:\n'BombMesh' is not found"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Override mesh
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->SetCustomMeshAsset(BombMesh);

	// Override material
	BombMaterialInternal = BombMesh->GetMaterial(0);
}

/*********************************************************************************************
 * Detonation
 ********************************************************************************************* */

// Sets the defaults of the bomb
void ABombActor::InitBomb(const APlayerCharacter* BombPlacer/* = nullptr*/)
{
	if (!HasAuthority()
	    || !MapComponentInternal)
	{
		return;
	}

	int32 InFireRadius = MIN_FIRE_RADIUS;
	int32 PlayerIndex = INDEX_NONE;
	if (BombPlacer) // Might be null if spawned from external source (e.g. cheat manager)
	{
		// Set bomb placer, so others can track who spawned the bomb, e.g: to record the score 
		BombPlacerInternal = BombPlacer;

		PlayerIndex = BombPlacer->GetPlayerId();
		InFireRadius = BombPlacer->GetPowerups().FireN;
		SetBombType(BombPlacer->GetPlayerType());
	}

	const UBombDataAsset& BombDataAsset = UBombDataAsset::Get();
	if (GetBombType() == ELevelType::None)
	{
		// Is bot character, set material for its default bomb with the same mesh
		const int32 BombMaterialsNum = BombDataAsset.GetBombMaterialsNum();
		if (PlayerIndex != INDEX_NONE // Is not debug character
		    && BombMaterialsNum)      // As least one bomb material
		{
			const int32 MaterialIndex = FMath::Abs(PlayerIndex) % BombMaterialsNum;
			BombMaterialInternal = BombDataAsset.GetBombMaterial(MaterialIndex);
		}
	}

	ApplyMaterial();

	FireRadiusInternal = InFireRadius;

	UpdateCollisionResponseToAllPlayers();

	TryDisplayExplosionCells();
}

// Returns cells that bombs is going to destroy
FCells ABombActor::GetExplosionCells() const
{
	if (!MapComponentInternal
	    || GetExplosionRadius() < MIN_FIRE_RADIUS)
	{
		return FCell::EmptyCells;
	}

	return UCellsUtilsLibrary::GetCellsAround(MapComponentInternal->GetCell(), EPathType::Explosion, GetExplosionRadius());
}

// Returns radius of the blast to each side
int32 ABombActor::GetExplosionRadius() const
{
#if !UE_BUILD_SHIPPING
	const int32 CheatOverride = UMyCheatManager::CVarBombRadius.GetValueOnAnyThread();
	if (CheatOverride > MIN_FIRE_RADIUS)
	{
		return CheatOverride;
	}
#endif //!UE_BUILD_SHIPPING

	return FireRadiusInternal;
}

// Show current explosion cells if the bomb type is allowed to be displayed, is not available in shipping build
void ABombActor::TryDisplayExplosionCells()
{
#if !UE_BUILD_SHIPPING
	FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = true;
	Params.TextColor = FLinearColor::Yellow;
	Params.TextSize += 50.f;
	Params.TextHeight += 1.f;
	UCellsUtilsLibrary::DisplayCells(this, GetExplosionCells(), Params);
#endif // !UE_BUILD_SHIPPING
}

// Destroy bomb and burst explosion cells, calls multicast event
void ABombActor::DetonateBomb()
{
	if (!HasAuthority()
	    || IsHidden()
	    || AMyGameStateBase::GetCurrentGameState() != ECGS::InGame)
	{
		return;
	}

	const FCells ExplosionCells = GetExplosionCells();
	if (!ensureMsgf(!ExplosionCells.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'ExplosionCells' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	MulticastDetonateBomb(ExplosionCells.Array());
}

// Destroy bomb and burst explosion cells
void ABombActor::MulticastDetonateBomb_Implementation(const FCellsArr& ExplosionCells)
{
	const UBombRow* BombRow = UBombDataAsset::Get().GetRowByLevelType<UBombRow>(GetBombType());
	if (!ensureMsgf(BombRow, TEXT("ASSERT: [%i] %hs:\n'BombRow' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Reset Fire Radius to avoid destroying the bomb again
	FireRadiusInternal = MIN_FIRE_RADIUS;

	// Spawn emitters
	ensureMsgf(BombRow->BombVFX, TEXT("ASSERT: [%i] %hs:\n'BombRow->BombVFX' is not set!"), __LINE__, __FUNCTION__);
	for (const FCell& Cell : ExplosionCells)
	{
		SpawnedVFXsInternal.Add(UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BombRow->BombVFX, Cell.Location, GetActorRotation(), GetActorScale()));
	}

	// Destroy all actors from array of cells
	AGeneratedMap::Get().DestroyLevelActorsOnCells(FCells{ExplosionCells}, this);

	USoundsSubsystem::Get().PlayExplosionSFX();

	auto OnVFXDurationExpired = [WeakThis = TWeakObjectPtr(this)]()
	{
		ABombActor* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		for (UNiagaraComponent* VfxIt : This->SpawnedVFXsInternal)
		{
			if (VfxIt)
			{
				VfxIt->Deactivate();
			}
		}

		This->SpawnedVFXsInternal.Empty();
	};

	constexpr bool bLoop = false;
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(TimerHandle_LifeSpanExpired);
	TimerManager.SetTimer(VFXDurationExpiredTimerHandle, OnVFXDurationExpired, UBombDataAsset::Get().GetVFXDuration(), bLoop);
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned.
void ABombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructBombActor();
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

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, FireRadiusInternal);
	DOREPLIFETIME(ThisClass, BombMaterialInternal);
	DOREPLIFETIME(ThisClass, BombPlacerInternal);
}

// Set the lifespan of this actor. When it expires the object will be destroyed
void ABombActor::SetLifeSpan(float InLifespan/* = DEFAULT_LIFESPAN*/)
{
	if (InLifespan == DEFAULT_LIFESPAN
	    && AMyGameStateBase::GetCurrentGameState() == ECGS::InGame)
	{
		InLifespan = UBombDataAsset::Get().GetLifeSpan();
	}

	// Don't call Super to allow its execution on clients

	InitialLifeSpan = InLifespan;

	// Initialize a timer for the actors lifespan if there is one. Otherwise clear any existing timer
	if (InLifespan > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_LifeSpanExpired, this, &AActor::LifeSpanExpired, InLifespan);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_LifeSpanExpired);
	}
}

// Called when the lifespan of an actor expires (if he has one)
void ABombActor::LifeSpanExpired()
{
	// Super::LifeSpanExpired() is not called here intentionally, because it destroys this actor directly 
	// Instead, call DestroyLevelActor from Generated Map to destroy it properly
	AGeneratedMap::Get().DestroyLevelActor(MapComponentInternal, this);
}

// Sets the actor to be hidden in the game. Alternatively used to avoid destroying
void ABombActor::SetActorHiddenInGame(bool bNewHidden)
{
	if (!bNewHidden)
	{
		// Is added on Generated Map

		ConstructBombActor();

		SetLifeSpan();

		// Binding to the event, that triggered when character end to overlaps the collision component
		OnActorEndOverlap.AddUniqueDynamic(this, &ABombActor::OnBombEndOverlap);

		// Listen when this bomb is destroyed on the Generated Map by itself or by other actors
		MapComponentInternal->OnDeactivatedMapComponent.AddUniqueDynamic(this, &ThisClass::OnDeactivatedMapComponent);
	}
	else
	{
		// Bomb is removed from Generated Map, detonate it

		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		MapComponentInternal->OnDeactivatedMapComponent.RemoveAll(this);

		OnActorEndOverlap.RemoveAll(this);

		BombMaterialInternal = nullptr;
		BombPlacerInternal = nullptr;
	}

	// Apply hidden flag
	Super::SetActorHiddenInGame(bNewHidden);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called on a bomb actor construction, could be called multiple times
void ABombActor::OnConstructionBombActor_Implementation()
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
	}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
}

// Triggers when character end to overlaps with this bomb.
void ABombActor::OnBombEndOverlap_Implementation(AActor* OverlappedActor, AActor* OtherActor)
{
	const bool bIsPlayerOverlap = Cast<APlayerCharacter>(OtherActor) != nullptr;
	if (!bIsPlayerOverlap)
	{
		return;
	}

	UpdateCollisionResponseToAllPlayers();
}

// Listen by dragged bombs to handle game resetting
void ABombActor::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	if (CurrentGameState == ECurrentGameState::InGame)
	{
		// Reinit bomb and restart lifespan
		InitBomb();
		SetLifeSpan();
	}
}

/*********************************************************************************************
 * Custom Collision Response
 ********************************************************************************************* */

// Called when owned map component is destroyed on the Generated Map
void ABombActor::OnDeactivatedMapComponent_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	// Bomb is removed from Generated Map, detonate it
	DetonateBomb();
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
		OnActorEndOverlap.RemoveAll(this);
	}
	else
	{
		// Add to bitmask overlapping players
		int32 Bitmask = 0;
		for (const AActor* OverlappingPlayerIt : OverlappingPlayers)
		{
			if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlappingPlayerIt))
			{
				Bitmask |= 1 << PlayerCharacter->GetPlayerId();
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

/*********************************************************************************************
 * Material
 ********************************************************************************************* */

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