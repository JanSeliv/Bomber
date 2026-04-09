// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrBombAbilityActor.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataRegistries/BmrBombRow.h"
#include "GameFramework/BmrGameState.h"
#include "MyUtilsLibraries/MultiplayerUtilsLibrary.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInstance.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrBombAbilityActor)

// Sets default values
ABmrBombAbilityActor::ABmrBombAbilityActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	static constexpr float NewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewUpdateFrequency);
	bAlwaysRelevant = true;
	SetReplicatingMovement(true);

	// Initialize Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// Initialize MapComponent
	MapComponent = CreateDefaultSubobject<UBmrMapComponent>(TEXT("MapComponent"));
}

// Returns true when bomb is fully initialized: both bomb is initialized and added to level
bool ABmrBombAbilityActor::IsBombReady() const
{
	return MapComponent && MapComponent->GetCell().IsValid() // Is added to level
	       && InstigatorAbilitySystemComponent; // Is initialized
}

/*********************************************************************************************
 * Detonation
 ********************************************************************************************* */

// Initiates the explosion: starts countdown and initializes the data (fire radius, explosion cells, etc.)
void ABmrBombAbilityActor::InitBomb(UAbilitySystemComponent* InASC)
{
	if (!ensureMsgf(InASC, TEXT("ASSERT: [%i] %hs:\n'InstigatorAbilitySystemComponent' is null, can not init the bomb!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	InstigatorAbilitySystemComponent = InASC;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstigatorAbilitySystemComponent, this);

	if (IsBombReady())
	{
		OnBombReady();
	}
}

// Returns explosion radius from instigator, or -1 if can not be obtained
int32 ABmrBombAbilityActor::GetFireRadius() const
{
	constexpr int32 DefaultFireRadius = 1;
	const UBmrPowerupsAttributeSet* PowerupsAttributeSet = UBmrPowerupsAttributeSet::GetPowerupsAttributeSet(InstigatorAbilitySystemComponent);
	return PowerupsAttributeSet ? PowerupsAttributeSet->GetPowerup_Fire() : DefaultFireRadius;
}

// Show current explosion cells if the bomb type is allowed to be displayed, is not available in shipping build
void ABmrBombAbilityActor::TryDisplayExplosionCells()
{
#if !UE_BUILD_SHIPPING
	FBmrDisplayCellsParams Params = FBmrDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = true;
	Params.TextColor = FLinearColor::Yellow;
	Params.TextSize += 50.f;
	Params.TextHeight += 1.f;
	UBmrCellUtilsLibrary::DisplayCells(this, ExplosionCells, Params);
#endif // !UE_BUILD_SHIPPING
}

// Calculates the explosion cells based on current fire radius
void ABmrBombAbilityActor::UpdateExplosionCells()
{
	if (!HasAuthority()
	    || !ExplosionCells.IsEmpty())
	{
		// Already calculated
		return;
	}

	ExplosionCells = UBmrCellUtilsLibrary::GetCellsAround(MapComponent->GetCell(), EPathType::Explosion, GetFireRadius());

	TryDisplayExplosionCells();
}

/*********************************************************************************************
 * Cue Visuals: VFXs, SFXs, Materials
 ********************************************************************************************* */

// Updates current mesh for this bomb actor, based on instigator type, or randomly if no instigator
void ABmrBombAbilityActor::ApplyMesh()
{
	const AActor* InstigatorActor = InstigatorAbilitySystemComponent ? InstigatorAbilitySystemComponent->GetAvatarActor() : nullptr;
	const FBmrBombRow& BombRow = FBmrBombRow::GetBombRow(InstigatorActor);
	UStreamableRenderAsset* BombMesh = BombRow.Mesh.Get();
	if (!ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %hs:\n'BombMesh' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->SetLocalMesh(BombMesh);
}

// Updates current material for this bomb actor, based on this bomb and Player placer types
void ABmrBombAbilityActor::ApplyMaterial()
{
	TObjectPtr<UMaterialInterface> NewBombMaterial = nullptr;

	// If bot character, override material with the player type
	const APawn* OwnedPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	const APlayerState* OwnerPlayerState = OwnedPawn ? OwnedPawn->GetPlayerState<APlayerState>() : nullptr;
	if (OwnerPlayerState && OwnedPawn->IsPlayerControlled())
	{
		// Set material by bomb type (default)
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		const UStaticMesh* BombMesh = MapComponent->GetMesh<UStaticMesh>();
		if (ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %hs:\n'BombMesh' is not found"), __LINE__, __FUNCTION__))
		{
			NewBombMaterial = BombMesh->GetMaterial(0);
		}
	}
	else if (const int32 BombMaterialsNum = FBmrBombRow::GetBombMaterialsNum())
	{
		// If bot character, set material for its default bomb with the same mesh
		const int32 PlayerIndex = OwnerPlayerState ? OwnerPlayerState->GetPlayerId() : FMath::RandRange(0, BombMaterialsNum - 1);
		const int32 MaterialIndex = FMath::Abs(PlayerIndex) % BombMaterialsNum;
		NewBombMaterial = FBmrBombRow::GetBombMaterial(MaterialIndex);
	}

	// Apply material
	if (NewBombMaterial)
	{
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
		MapComponent->SetLocalMeshMaterial(NewBombMaterial);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned.
void ABmrBombAbilityActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	ABmrGeneratedMap::Get().AddToGrid(MapComponent);
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABmrBombAbilityActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InstigatorAbilitySystemComponent, Params);
}

// Called on client to init bomb on clients when instigator's ASC is replicated
void ABmrBombAbilityActor::OnRep_InstigatorAbilitySystemComponent()
{
	if (InstigatorAbilitySystemComponent)
	{
		InitBomb(InstigatorAbilitySystemComponent);
	}
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
void ABmrBombAbilityActor::OnAddedToLevel_Implementation(UBmrMapComponent* InMapComponent)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	if (IsBombReady())
	{
		OnBombReady();
	}

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UBmrUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when bomb is fully initialized: both cell is valid and instigator ASC is set
void ABmrBombAbilityActor::OnBombReady_Implementation()
{
	UpdateExplosionCells();

	ApplyMesh();

	ApplyMaterial();

	// Initialize collision, waiting for SROC to reach bomb cell if needed
	const bool bIsWaiting = TryBindOnInstigatorReachedBombCell();
	if (!bIsWaiting)
	{
		// Not waiting, can init collision immediately
		InitCollisionResponseToAllPlayers();
	}
}

// Is used for cleaning up the bomb's data after it was removed from the level
void ABmrBombAbilityActor::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.RemoveAll(this);

	// Clear all cell delegates this bomb subscribed to
	FMapComponents AllPlayerMapComponents;
	UBmrActorUtilsLibrary::GetLevelActors(AllPlayerMapComponents, TO_FLAG(EAT::Player));
	for (UBmrMapComponent* PlayerMapComponentIt : AllPlayerMapComponents)
	{
		checkf(PlayerMapComponentIt, TEXT("ERROR: [%i] %hs:\n'PlayerMapComponentIt' is null!"), __LINE__, __FUNCTION__);
		PlayerMapComponentIt->OnCellChanged.RemoveAll(this);
	}

	const APawn* InstigatorPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	if (InstigatorPawn)
	{
		InstigatorPawn->GetWorldTimerManager().ClearAllTimersForObject(this);
	}

	InstigatorAbilitySystemComponent = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstigatorAbilitySystemComponent, this);

	ExplosionCells = FBmrCell::EmptyCells;
}

// Is called when character leaves the bomb to update collision response
void ABmrBombAbilityActor::OnAnyPawnCellExit_Implementation(UBmrMapComponent* PlayerMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	const UBoxComponent* BoxCollisionComponent = MapComponent->GetBoxCollisionComponent();
	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	FCollisionResponseContainer CollisionResponses = BoxCollisionComponent->GetCollisionResponseToChannels();

	// true: player left the bomb, block collision (all other channels and players will stay as they are)
	// false: client player with high ping might be teleported back into blocked bomb, so release collision
	const bool bIsPlayerLeft = NewCell != MapComponent->GetCell();
	const ECollisionResponse NewResponse = bIsPlayerLeft ? ECR_Block : ECR_Overlap;

	checkf(PlayerMapComponent, TEXT("ERROR: [%i] %hs:\n'PlayerMapComponent' is null!"), __LINE__, __FUNCTION__);
	PlayerMapComponent->OnCellChanged.RemoveAll(this);

	const APawn* Pawn = PlayerMapComponent ? PlayerMapComponent->GetOwner<APawn>() : nullptr;
	const APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<APlayerState>() : nullptr;
	const int32 PlayerId = PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;

	GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, PlayerId, NewResponse);
	MapComponent->SetCollisionResponses(CollisionResponses);
}

/*********************************************************************************************
 * Custom Collision Response
 ********************************************************************************************* */

// Returns true if this bomb was spawned for server replica of client (SROC), meaning server is processing bomb placed by client
bool ABmrBombAbilityActor::IsServerReplicaOfClient() const
{
	const APawn* InstigatorPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	return HasAuthority()
	       && InstigatorPawn
	       && !InstigatorPawn->IsLocallyControlled();
}

// Sets actual collision responses to all players for this bomb
void ABmrBombAbilityActor::InitCollisionResponseToAllPlayers()
{
	const APawn* InstigatorPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	if (UBmrMapComponent* InstigatorMapComponent = UBmrMapComponent::GetMapComponent(InstigatorPawn))
	{
		InstigatorMapComponent->OnCellChanged.RemoveAll(this);
	}

	// Obtain all overlapped level actors on the bomb cell to enable overlap response for players inside the bomb
	FMapComponents OverlapMapComponents;
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	UBmrActorUtilsLibrary::GetLevelActorsOnCells(/*out*/ OverlapMapComponents, {MapComponent->GetCell()});

	// Obtain default collision responses
	const UBoxComponent* BoxCollisionComponent = MapComponent ? MapComponent->GetBoxCollisionComponent() : nullptr;
	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	FCollisionResponseContainer CollisionResponses = BoxCollisionComponent->GetCollisionResponseToChannels();

	// Block all players by default (all non-player channels will remain unchanged)
	static constexpr int32 MaxPlayerID = 3;
	for (int32 PlayerId = 0; PlayerId <= MaxPlayerID; ++PlayerId)
	{
		GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, PlayerId, ECR_Block);
	}

	// Unlock (allow overlap) those players which overlap with this bomb
	for (const UBmrMapComponent* OverlapMapComponentIt : OverlapMapComponents)
	{
		const APawn* Pawn = OverlapMapComponentIt ? OverlapMapComponentIt->GetOwner<APawn>() : nullptr;
		const APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<APlayerState>() : nullptr;
		if (!PlayerState)
		{
			// Is different overlapped actor, likely bomb itself
			continue;
		}

		// Change response for the player which overlaps with this bomb (all other channels and players will stay as they are)
		GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, PlayerState->GetPlayerId(), ECR_Overlap);

		// Listen when character end to overlaps with this bomb to block collision
		UBmrMapComponent* PlayerMapComponent = UBmrMapComponent::GetMapComponent(Pawn);
		checkf(PlayerMapComponent, TEXT("ERROR: [%i] %hs:\n'PlayerMapComponent' is null!"), __LINE__, __FUNCTION__);
		PlayerMapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnAnyPawnCellExit);
	}

	MapComponent->SetCollisionResponses(CollisionResponses);
}

// Takes your container and returns is with new specified response for player by its specified ID
void ABmrBombAbilityActor::GetCollisionResponseToPlayerByID(FCollisionResponseContainer& InOutCollisionResponses, int32 PlayerId, ECollisionResponse NewResponse)
{
	if (PlayerId < 0)
	{
		return;
	}

	ECollisionChannel CollisionChannel = ECC_WorldDynamic;
	switch (PlayerId)
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

// Attempts to bind bomb to wait for instigator pawn to reach bomb cell before initializing collision
bool ABmrBombAbilityActor::TryBindOnInstigatorReachedBombCell()
{
	if (!IsServerReplicaOfClient())
	{
		// Not SROC, no need to wait and can init collision immediately
		return false;
	}

	const APawn* InstigatorPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	UBmrMapComponent* InstigatorMapComponent = UBmrMapComponent::GetMapComponent(InstigatorPawn);
	const FBmrCell CurrentPawnCell = InstigatorMapComponent ? InstigatorMapComponent->GetCell() : FBmrCell::InvalidCell;
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	if (!CurrentPawnCell.IsValid()
	    || CurrentPawnCell == MapComponent->GetCell())
	{
		// Is already at the bomb cell, no need to wait
		return false;
	}

	// Wait for pawn to reach the bomb cell before initializing collision
	InstigatorMapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnInstigatorPawnCellEnter);

	// Set timeout of waiting
	FTimerHandle BombSpawnTimeoutHandle;
	const float MaxPingCompensationSec = UBmrGameStateDataAsset::Get().GetMaxPingCompensationSec();
	constexpr float TimeoutBuffer = 1.5f;
	const float PlayerPingSeconds = UMultiplayerUtilsLibrary::GetPlayerPingSeconds(InstigatorPawn);
	const float TimeoutSeconds = FMath::Min(PlayerPingSeconds * TimeoutBuffer, MaxPingCompensationSec);
	InstigatorPawn->GetWorldTimerManager().SetTimer(BombSpawnTimeoutHandle, this, &ThisClass::InitCollisionResponseToAllPlayers, TimeoutSeconds);

	return true;
}

// Called when instigator pawn reaches the bomb cell, initializes collision
void ABmrBombAbilityActor::OnInstigatorPawnCellEnter_Implementation(UBmrMapComponent* InMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	if (NewCell != MapComponent->GetCell())
	{
		return;
	}

	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'InMapComponent' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnCellChanged.RemoveAll(this);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	InitCollisionResponseToAllPlayers();
}
