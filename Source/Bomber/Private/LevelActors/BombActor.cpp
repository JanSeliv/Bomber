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
	if (IS_TRANSIENT(this)                 // This actor is transient
	    || !IsValid(MapComponentInternal)) // Is not valid for map construction
	{
		return;
	}

	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->OnOwnerWantsReconstruct.AddUniqueDynamic(this, &ThisClass::OnConstructionBombActor);
	MapComponentInternal->ConstructOwnerActor();

	// Init bomb by default, if it's spawned from player, it will be reinitialized again
	InitBomb();

	// Start countdown to destroy the bomb
	SetLifeSpan();

	// Binding to the event, that triggered when character end to overlaps the collision component
	OnActorEndOverlap.AddUniqueDynamic(this, &ThisClass::OnBombEndOverlap);

	// Listen when this bomb is destroyed on the Generated Map by itself or by other actors
	MapComponentInternal->OnPreRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPreRemovedFromLevel);

	// Listen for client to react when bomb is reset to play explosions cue
	MapComponentInternal->OnActorTypeChanged.AddUniqueDynamic(this, &ThisClass::OnActorTypeChanged);

	// Listen for client to react when bomb is added to the level
	MapComponentInternal->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnCellChanged);

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
	}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
}

//  Returns the type of the bomb
ELevelType ABombActor::GetBombType() const
{
	return MapComponentInternal ? MapComponentInternal->GetLevelType() : ELevelType::None;
}

/*********************************************************************************************
 * Detonation
 ********************************************************************************************* */

// Sets the defaults of the bomb
void ABombActor::InitBomb(const APlayerCharacter* BombPlacer/* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	constexpr int32 MinFireRadius = 1;
	int32 InFireRadius = MinFireRadius;
	if (BombPlacer) // Might be null if spawned from external source (e.g. cheat manager)
	{
		// Set bomb placer, so others can track who spawned the bomb, e.g: to record the score 
		BombPlacerInternal = BombPlacer;

		InFireRadius = BombPlacer->GetPowerups().FireN;

		// Override default mesh with one with the player type (each character has own bomb)
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		const ULevelActorRow* BombRow = UBombDataAsset::Get().GetRowByLevelType(BombPlacer->GetPlayerType());
		MapComponentInternal->SetMesh(BombRow->Mesh);
	}

#if !UE_BUILD_SHIPPING
	const int32 CheatOverride = UMyCheatManager::CVarBombRadius.GetValueOnAnyThread();
	if (CheatOverride > MinFireRadius)
	{
		InFireRadius = CheatOverride;
	}
#endif //!UE_BUILD_SHIPPING

	// Set fire radius (from player, cheat manager or default) and update explosion cells
	FireRadiusInternal = InFireRadius;
	UpdateExplosionCells();

	ApplyMaterial();

	UpdateCollisionResponseToAllPlayers();
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
	UCellsUtilsLibrary::DisplayCells(this, FCells{LocalExplosionCellsInternal}, Params);
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

	if (!ensureMsgf(!LocalExplosionCellsInternal.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'LocalExplosionCellsInternal' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Start countdown to destroy the bomb
	SetLifeSpan();

	// Reset Fire Radius to avoid destroying the bomb again
	FireRadiusInternal = 0;

	PlayExplosionsCue();

	// Destroy all actors from array of cells
	AGeneratedMap::Get().DestroyLevelActorsOnCells(FCells{LocalExplosionCellsInternal}, this);

	USoundsSubsystem::Get().PlayExplosionSFX();
}

// Calculates the explosion cells based on current fire radius
void ABombActor::UpdateExplosionCells()
{
	if (!IsValid(MapComponentInternal)
	    || MapComponentInternal->GetCell().IsInvalidCell()
	    || FireRadiusInternal <= 0)
	{
		// On client some data might be not replicated yet
		return;
	}

	LocalExplosionCellsInternal = UCellsUtilsLibrary::GetCellsAround(MapComponentInternal->GetCell(), EPathType::Explosion, FireRadiusInternal);

	TryDisplayExplosionCells();
}

// Is called on client to update current bomb placer
void ABombActor::OnRep_BombPlacer()
{
	if (BombPlacerInternal)
	{
		ApplyMaterial();
	}
}

// Is called on client to recalculate the explosion cells
void ABombActor::OnRep_FireRadius()
{
	if (FireRadiusInternal > 0
	    && MapComponentInternal
	    && MapComponentInternal->GetCell().IsValid())
	{
		UpdateExplosionCells();
	}
}

/*********************************************************************************************
 * Cue Visuals: VFXs, SFXs, Materials
 ********************************************************************************************* */

// Spawns VFXs and SFXs, is allowed to call both on server and clients
void ABombActor::PlayExplosionsCue(const UBombRow* BombRow/* = nullptr*/)
{
	if (AMyGameStateBase::GetCurrentGameState() != ECGS::InGame
	    || !ensureMsgf(!LocalExplosionCellsInternal.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'LocalExplosionCellsInternal' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (!BombRow)
	{
		// Row is optional, if null, then it's taken from current bomb type
		BombRow = UBombDataAsset::Get().GetRowByLevelType<UBombRow>(GetBombType());
	}

	UNiagaraSystem* BombVFX = BombRow ? BombRow->BombVFX : nullptr;
	if (!ensureMsgf(BombVFX, TEXT("ASSERT: [%i] %hs:\n'BombVFX' is not set!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Play SFX
	USoundsSubsystem::Get().PlayExplosionSFX();

	// Spawn VFXs
	for (const FCell& Cell : LocalExplosionCellsInternal)
	{
		SpawnedVFXsInternal.Add(UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BombVFX, Cell.Location, GetActorRotation(), GetActorScale()));
	}

	// Stop VFXs after the duration, so they all have the same duration
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

// Updates current material for this bomb actor, based on this bomb and Player placer types
void ABombActor::ApplyMaterial()
{
	TObjectPtr<class UMaterialInterface> NewBombMaterial = nullptr;

	// If bot character, override material with the player type
	if (BombPlacerInternal
	    && BombPlacerInternal->IsBotControlled())
	{
		// If bot character, set material for its default bomb with the same mesh
		const int32 PlayerIndex = BombPlacerInternal->GetPlayerId();
		const UBombDataAsset& BombDataAsset = UBombDataAsset::Get();
		const int32 BombMaterialsNum = BombDataAsset.GetBombMaterialsNum();
		if (PlayerIndex != INDEX_NONE // Is not debug character
		    && BombMaterialsNum)      // As least one bomb material
		{
			const int32 MaterialIndex = FMath::Abs(PlayerIndex) % BombMaterialsNum;
			NewBombMaterial = BombDataAsset.GetBombMaterial(MaterialIndex);
		}
	}
	else
	{
		// Set material by bomb type (default)
		const ULevelActorRow* BombRow = UBombDataAsset::Get().GetRowByLevelType(GetBombType());
		const UStaticMesh* BombMesh = BombRow ? Cast<UStaticMesh>(BombRow->Mesh) : nullptr;
		if (ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %hs:\n'BombMesh' is not found"), __LINE__, __FUNCTION__))
		{
			NewBombMaterial = BombMesh->GetMaterial(0);
		}
	}

	// Apply material
	if (NewBombMaterial)
	{
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		MapComponentInternal->SetMaterial(NewBombMaterial);
	}
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

// Returns properties that are replicated for the lifetime of the actor channel
void ABombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, FireRadiusInternal);
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
	// Super::LifeSpanExpired() is not called here intentionally, because bomb actor shouldn't be destroyed directly, but handled by its own
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
	}

	// Apply hidden flag
	Super::SetActorHiddenInGame(bNewHidden);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

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

// Called when owned map component is destroyed on the Generated Map
void ABombActor::OnPreRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	if (HasAuthority())
	{
		// On server, bomb is removed from Generated Map, detonate it
		DetonateBomb();
	}

	if (MapComponentInternal)
	{
		MapComponentInternal->OnPreRemovedFromLevel.RemoveAll(this);
		MapComponentInternal->OnActorTypeChanged.RemoveAll(this);
		MapComponentInternal->OnCellChanged.RemoveAll(this);
	}

	OnActorEndOverlap.RemoveAll(this);

	BombPlacerInternal = nullptr;

	LocalExplosionCellsInternal = FCell::EmptyCells;
}

// Listen when bomb changed visuals
void ABombActor::OnActorTypeChanged_Implementation(UMapComponent* MapComponent, const class ULevelActorRow* NewRow, const class ULevelActorRow* PreviousRow)
{
	const bool bIsBombReset = !NewRow && PreviousRow;
	if (bIsBombReset
	    && !HasAuthority())
	{
		// The bomb was just reset on client, play visuals from Previous Row, which was reset 
		PlayExplosionsCue(CastChecked<UBombRow>(PreviousRow));
	}
}

// Is used on client to react when bomb is added to the level
void ABombActor::OnCellChanged_Implementation(UMapComponent* MapComponent, const FCell& NewCell, const FCell& PreviousCell)
{
	if (NewCell.IsValid()
	    && !HasAuthority())
	{
		// On client, the bomb was just added to the level, update cells
		UpdateExplosionCells();
	}
}

/*********************************************************************************************
 * Custom Collision Response
 ********************************************************************************************* */

// Sets actual collision responses to all players for this bomb
void ABombActor::UpdateCollisionResponseToAllPlayers()
{
	const UBoxComponent* BoxCollisionComponent = MapComponentInternal ? MapComponentInternal->GetBoxCollisionComponent() : nullptr;
	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	FCollisionResponseContainer CollisionResponses = BoxCollisionComponent->GetCollisionResponseToChannels();

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