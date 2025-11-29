// Copyright (c) Yevhenii Selivanov.

#include "LevelActors/BombActor.h"

// Bomber
#include "AbilitySystem/Attributes/BmrPowerupsAttributeSet.h"
#include "Bomber.h"
#include "Components/MapComponent.h"
#include "DataAssets/BombDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "GeneratedMap.h"
#include "Structures/BmrGameplayTags.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"

#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#include "MyUnrealEdEngine.h"
#endif

// UE
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

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

/*********************************************************************************************
 * Detonation
 ********************************************************************************************* */

// Initiates the explosion: starts countdown and initializes the data (fire radius, explosion cells, etc.)
void ABombActor::InitBomb(UAbilitySystemComponent* InASC)
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
int32 ABombActor::GetFireRadius() const
{
	constexpr int32 DefaultFireRadius = 1;
	const UBmrPowerupsAttributeSet* PowerupsAttributeSet = UBmrPowerupsAttributeSet::GetPowerupsAttributeSet(InstigatorAbilitySystemComponent);
	return PowerupsAttributeSet ? PowerupsAttributeSet->GetPowerup_Fire() : DefaultFireRadius;
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
	UCellsUtilsLibrary::DisplayCells(this, ExplosionCellsInternal, Params);
#endif // !UE_BUILD_SHIPPING
}

// Calculates the explosion cells based on current fire radius
void ABombActor::UpdateExplosionCells()
{
	if (!HasAuthority()
	    || !ExplosionCellsInternal.IsEmpty())
	{
		// Already calculated
		return;
	}

	ExplosionCellsInternal = UCellsUtilsLibrary::GetCellsAround(MapComponentInternal->GetCell(), EPathType::Explosion, GetFireRadius());

	TryDisplayExplosionCells();
}

/*********************************************************************************************
 * Cue Visuals: VFXs, SFXs, Materials
 ********************************************************************************************* */

// Updates current mesh for this bomb actor, based on instigator type, or randomly if no instigator
void ABombActor::ApplyMesh()
{
	const ULevelActorRow* BombRow = InstigatorAbilitySystemComponent ? UBombDataAsset::Get().GetBombRow(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	if (!ensureMsgf(BombRow, TEXT("ASSERT: [%i] %hs:\n'BombRow' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->SetLocalMesh(BombRow->Mesh);
}

// Updates current material for this bomb actor, based on this bomb and Player placer types
void ABombActor::ApplyMaterial()
{
	TObjectPtr<UMaterialInterface> NewBombMaterial = nullptr;

	// If bot character, override material with the player type
	const APawn* OwnedPawn = InstigatorAbilitySystemComponent ? Cast<APawn>(InstigatorAbilitySystemComponent->GetAvatarActor()) : nullptr;
	const APlayerState* OwnerPlayerState = OwnedPawn ? OwnedPawn->GetPlayerState<APlayerState>() : nullptr;
	if (OwnerPlayerState && OwnedPawn->IsPlayerControlled())
	{
		// Set material by bomb type (default)
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		const UStaticMesh* BombMesh = MapComponentInternal->GetMesh<UStaticMesh>();
		if (ensureMsgf(BombMesh, TEXT("ASSERT: [%i] %hs:\n'BombMesh' is not found"), __LINE__, __FUNCTION__))
		{
			NewBombMaterial = BombMesh->GetMaterial(0);
		}
	}
	else if (const int32 BombMaterialsNum = UBombDataAsset::Get().GetBombMaterialsNum())
	{
		// If bot character, set material for its default bomb with the same mesh
		const int32 PlayerIndex = OwnerPlayerState ? OwnerPlayerState->GetPlayerId() : FMath::RandRange(0, BombMaterialsNum - 1);
		const int32 MaterialIndex = FMath::Abs(PlayerIndex) % BombMaterialsNum;
		NewBombMaterial = UBombDataAsset::Get().GetBombMaterial(MaterialIndex);
	}

	// Apply material
	if (NewBombMaterial)
	{
		checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
		MapComponentInternal->SetLocalMeshMaterial(NewBombMaterial);
	}
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

// Called when an instance of this class is placed (in editor) or spawned.
void ABombActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BIND_ON_ADDED_TO_LEVEL(this, ThisClass::OnAddedToLevel);
	AGeneratedMap::Get().AddToGrid(MapComponentInternal);
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InstigatorAbilitySystemComponent, Params);
}

// Called on client to init bomb on clients when instigator's ASC is replicated
void ABombActor::OnRep_InstigatorAbilitySystemComponent()
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
void ABombActor::OnAddedToLevel_Implementation(UMapComponent* MapComponent)
{
	checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
	MapComponent->OnPostRemovedFromLevel.AddUniqueDynamic(this, &ThisClass::OnPostRemovedFromLevel);

	InitCollisionResponseToAllPlayers();

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld()) // [IsEditorNotPieWorld]
	{
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// Is used for cleaning up the bomb's data after it was removed from the level
void ABombActor::OnPostRemovedFromLevel_Implementation(UMapComponent* MapComponent, UObject* DestroyCauser)
{
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	MapComponentInternal->OnPostRemovedFromLevel.RemoveAll(this);
	MapComponentInternal->OnCellChanged.RemoveAll(this);

	InstigatorAbilitySystemComponent = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InstigatorAbilitySystemComponent, this);

	ExplosionCellsInternal = FCell::EmptyCells;
}

// Is called when character leaves the bomb to update collision response
void ABombActor::OnPlayerCellChanged_Implementation(UMapComponent* PlayerMapComponent, const FCell& NewCell, const FCell& PreviousCell)
{
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	const UBoxComponent* BoxCollisionComponent = MapComponentInternal->GetBoxCollisionComponent();
	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	FCollisionResponseContainer CollisionResponses = BoxCollisionComponent->GetCollisionResponseToChannels();

	// true: player left the bomb, block collision (all other channels and players will stay as they are)
	// false: client player with high ping might be teleported back into blocked bomb, so release collision
	const bool bIsPlayerLeft = NewCell != MapComponentInternal->GetCell();
	const ECollisionResponse NewResponse = bIsPlayerLeft ? ECR_Block : ECR_Overlap;

	checkf(PlayerMapComponent, TEXT("ERROR: [%i] %hs:\n'PlayerMapComponent' is null!"), __LINE__, __FUNCTION__);
	const APawn* Pawn = PlayerMapComponent ? PlayerMapComponent->GetOwner<APawn>() : nullptr;
	const APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<APlayerState>() : nullptr;
	const int32 PlayerId = PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;

	GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, PlayerId, NewResponse);
	MapComponentInternal->SetCollisionResponses(CollisionResponses);
}

/*********************************************************************************************
 * Custom Collision Response
 ********************************************************************************************* */

// Sets actual collision responses to all players for this bomb
void ABombActor::InitCollisionResponseToAllPlayers()
{
	// Obtain all overlapped level actors on the bomb cell to enable overlap response for players inside the bomb
	FMapComponents OverlapMapComponents;
	checkf(MapComponentInternal, TEXT("ERROR: [%i] %hs:\n'MapComponentInternal' is null!"), __LINE__, __FUNCTION__);
	ULevelActorsUtilsLibrary::GetLevelActorsOnCells(/*out*/ OverlapMapComponents, {MapComponentInternal->GetCell()});

	// Obtain default collision responses
	const UBoxComponent* BoxCollisionComponent = MapComponentInternal ? MapComponentInternal->GetBoxCollisionComponent() : nullptr;
	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	FCollisionResponseContainer CollisionResponses = BoxCollisionComponent->GetCollisionResponseToChannels();

	// Block all players by default (all non-player channels will remain unchanged)
	static constexpr int32 MaxPlayerID = 3;
	for (int32 CharacterID = 0; CharacterID <= MaxPlayerID; ++CharacterID)
	{
		GetCollisionResponseToPlayerByID(/*InOut*/ CollisionResponses, CharacterID, ECR_Block);
	}

	// Unlock (allow overlap) those players which overlap with this bomb
	for (const UMapComponent* OverlapMapComponentIt : OverlapMapComponents)
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
		UMapComponent* PlayerMapComponent = UMapComponent::GetMapComponent(Pawn);
		checkf(PlayerMapComponent, TEXT("ERROR: [%i] %hs:\n'PlayerMapComponent' is null!"), __LINE__, __FUNCTION__);
		PlayerMapComponent->OnCellChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerCellChanged);
	}

	MapComponentInternal->SetCollisionResponses(CollisionResponses);
}

// Takes your container and returns is with new specified response for player by its specified ID
void ABombActor::GetCollisionResponseToPlayerByID(FCollisionResponseContainer& InOutCollisionResponses, int32 CharacterID, ECollisionResponse NewResponse)
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