// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrBombAbilityActor.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "Components/BmrMapComponent.h"
#include "DataAssets/BmrBombDataAsset.h"
#include "GameFramework/BmrGameState.h"
#include "Structures/BmrGameplayTags.h"
#include "UtilityLibraries/BmrActorUtilsLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

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

	UpdateExplosionCells();

	ApplyMesh();

	ApplyMaterial();
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
	const UBmrBombRow* BombRow = InstigatorAbilitySystemComponent ? UBmrBombDataAsset::Get().GetBombRow(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	if (!ensureMsgf(BombRow, TEXT("ASSERT: [%i] %hs:\n'BombRow' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponent->SetLocalMesh(BombRow->Mesh);
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
	else if (const int32 BombMaterialsNum = UBmrBombDataAsset::Get().GetBombMaterialsNum())
	{
		// If bot character, set material for its default bomb with the same mesh
		const int32 PlayerIndex = OwnerPlayerState ? OwnerPlayerState->GetPlayerId() : FMath::RandRange(0, BombMaterialsNum - 1);
		const int32 MaterialIndex = FMath::Abs(PlayerIndex) % BombMaterialsNum;
		NewBombMaterial = UBmrBombDataAsset::Get().GetBombMaterial(MaterialIndex);
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

	InitCollisionResponseToAllPlayers();

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UBmrUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// Is used for cleaning up the bomb's data after it was removed from the level
void ABmrBombAbilityActor::OnPostRemovedFromLevel_Implementation(UBmrMapComponent* InMapComponent, UObject* DestroyCauser)
{
	checkf(InMapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	InMapComponent->OnPostRemovedFromLevel.RemoveAll(this);
	InMapComponent->OnCellChanged.RemoveAll(this);

	InstigatorAbilitySystemComponent = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstigatorAbilitySystemComponent, this);

	ExplosionCells = FBmrCell::EmptyCells;
}

// Is called when character leaves the bomb to update collision response
void ABmrBombAbilityActor::OnPlayerCellChanged_Implementation(UBmrMapComponent* PlayerMapComponent, const FBmrCell& NewCell, const FBmrCell& PreviousCell)
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
	const APawn* Pawn = PlayerMapComponent ? PlayerMapComponent->GetOwner<APawn>() : nullptr;
	const APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<APlayerState>() : nullptr;
	const int32 PlayerId = PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;

	GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, PlayerId, NewResponse);
	MapComponent->SetCollisionResponses(CollisionResponses);
}

/*********************************************************************************************
 * Custom Collision Response
 ********************************************************************************************* */

// Sets actual collision responses to all players for this bomb
void ABmrBombAbilityActor::InitCollisionResponseToAllPlayers()
{
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
		PlayerMapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerCellChanged);
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