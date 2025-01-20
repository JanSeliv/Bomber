// Copyright (c) Yevhenii Selivanov.

#include "Components/MapComponent.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "PoolManagerSubsystem.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/LevelActorDataAsset.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/BoxComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/StreamableRenderAsset.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MapComponent)

// Returns the map component of the specified owner
UMapComponent* UMapComponent::GetMapComponent(const AActor* Owner)
{
	return Owner ? Owner->FindComponentByClass<UMapComponent>() : nullptr;
}

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Replicate a component
	SetIsReplicatedByDefault(true);

	// Initialize the Box Collision component
	BoxCollisionComponentInternal = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
}

// Rerun owner's construction scripts. The temporary only editor owner will not be updated
void UMapComponent::ConstructOwnerActor()
{
	// Construct the actor's map component
	const bool bIsConstructed = OnConstructionOwnerActor();
	if (!bIsConstructed)
	{
		return;
	}

	if (OnOwnerWantsReconstruct.IsBound())
	{
		OnOwnerWantsReconstruct.Broadcast();
	}
}

/*********************************************************************************************
 * Cell (Location)
 ********************************************************************************************* */

// Override current cell data, where owner is located on the Generated Map
void UMapComponent::SetCell(const FCell& Cell)
{
	CellInternal = Cell;

	TryDisplayOwnedCell();
}

// Show current cell if owned actor type is allowed, is not available in shipping build
void UMapComponent::TryDisplayOwnedCell(bool bClearPrevious/* = false*/)
{
#if !UE_BUILD_SHIPPING
	FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = bClearPrevious
	                                || UUtilsLibrary::IsEditorNotPieWorld(); // Always clear before PIE, so it properly updates when uncheck bShouldShowRenders
	UCellsUtilsLibrary::DisplayCell(GetOwner(), CellInternal, Params);
#endif // !UE_BUILD_SHIPPING
}

/*********************************************************************************************
 * Mesh
 ********************************************************************************************* */

// Returns current mesh asset
class UStreamableRenderAsset* UMapComponent::GetMesh() const
{
	return UGameplayUtilsLibrary::GetMesh(MeshComponentInternal);
}

// Applies given mesh on owner actor, or resets the mesh if null is passed
void UMapComponent::SetMesh(UStreamableRenderAsset* NewMesh)
{
	if (GetMesh() == NewMesh                                                // is already set
	    || GetActorDataAssetChecked().GetActorType() == EActorType::Player) // ACharacter has own mesh component, no need to manage it
	{
		return;
	}

	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);

	const UStreamableRenderAsset* PreviousMesh = GetMesh();
	UGameplayUtilsLibrary::SetMesh(MeshComponentInternal, NewMesh);

	if (Owner->HasAuthority())
	{
		const ULevelActorRow* PreviousRow = GetActorDataAssetChecked().GetRowByMesh(PreviousMesh);
		const ULevelActorRow* NewRow = NewMesh ? GetActorDataAssetChecked().GetRowByMesh(NewMesh) : nullptr;

		// On server, set the index of the mesh, is primarily used for replicating the mesh to the clients, it's INDEX_NONE when reset the mesh (null is passed)
		const int32 NewRowIndex = NewRow ? ActorDataAssetInternal->GetIndexByRow(NewRow) : INDEX_NONE;
		ensureMsgf(!NewMesh || NewRowIndex != INDEX_NONE, TEXT("ASSERT: [%i] %hs:\nAttempted to set the '%s' mesh on '%s' actor that is not stored in the '%s' data asset!"), __LINE__, __FUNCTION__, *GetNameSafe(NewMesh), *GetNameSafe(Owner), *GetNameSafe(ActorDataAssetInternal));
		RowIndexInternal = NewRowIndex;

		// On server, broadcast event; clients will get notify later by replicating the RowIndex
		OnActorTypeChanged.Broadcast(this, NewRow, PreviousRow);
	}
}

/** Set material to the mesh. */
void UMapComponent::SetMaterial(UMaterialInterface* Material)
{
	if (MeshComponentInternal
	    && Material)
	{
		MeshComponentInternal->SetMaterial(0, Material);
	}
}

// Is called on client to update current level actor row
void UMapComponent::OnRep_RowIndex(int32 PreviousRowIndex)
{
	const ULevelActorDataAsset& ActorDataAsset = GetActorDataAssetChecked();
	const ULevelActorRow* PreviousRow = ActorDataAsset.GetRowByIndex(PreviousRowIndex);
	const ULevelActorRow* NewRow = ActorDataAsset.GetRowByIndex(RowIndexInternal);

	// The mesh might be null, then perform the cleanup
	UStreamableRenderAsset* NewMesh = NewRow ? NewRow->Mesh : nullptr;
	SetMesh(NewMesh);

	// On client, broadcast event post RowIndex replication
	OnActorTypeChanged.Broadcast(this, NewRow, PreviousRow);

	if (RowIndexInternal == INDEX_NONE)
	{
		// It's client which invalidated mesh, broadcast events manually as initial removal happened only on the server
		OnPreRemoved();
		OnPostRemoved();
	}
}

/*********************************************************************************************
 * Collision
 ********************************************************************************************* */

// Set new collisions data for any channel of the Box Collision Component
void UMapComponent::SetCollisionResponses(const FCollisionResponseContainer& NewResponses)
{
	const AActor* Owner = GetOwner();
	if (!Owner
	    || !Owner->HasAuthority()
	    || NewResponses == ECR_MAX
	    || NewResponses == CollisionResponseInternal)
	{
		return;
	}

	CollisionResponseInternal = NewResponses;
	ApplyCollisionResponse();
}

// Updates current collisions for the Box Collision Component
void UMapComponent::ApplyCollisionResponse()
{
	if (!BoxCollisionComponentInternal
	    || CollisionResponseInternal == ECR_MAX)
	{
		return;
	}

	BoxCollisionComponentInternal->SetCollisionResponseToChannels(CollisionResponseInternal);
}

// Is called on client to response on changes in collision responses
void UMapComponent::OnRep_CollisionResponse()
{
	ApplyCollisionResponse();
}

/*********************************************************************************************
 * Data Asset
 ********************************************************************************************* */

// Get the owner's data asset
const ULevelActorDataAsset& UMapComponent::GetActorDataAssetChecked() const
{
	checkf(ActorDataAssetInternal, TEXT("EROR: 'ActorDataAssetInternal' is null"));
	return *ActorDataAssetInternal;
}

// Get the owner's data asset
EActorType UMapComponent::GetActorType() const
{
	return GetActorDataAssetChecked().GetActorType();
}

// Returns the level type by current mesh
ELevelType UMapComponent::GetLevelType() const
{
	const ULevelActorRow* FoundRow = GetActorDataAssetChecked().GetRowByIndex(RowIndexInternal);
	return FoundRow ? FoundRow->LevelType : ELevelType::None;
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

//  Called when a component is registered (not loaded)
void UMapComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	check(Owner);

	// Register level actors for game features only if the game was started
	constexpr bool bAddOnlyInGameWorlds = true;
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(Owner, bAddOnlyInGameWorlds);

	if (ActorDataAssetInternal)
	{
		// Its data asset is valid, so initialization was already performed before
		return;
	}

	// Set the tick disabled by default and decrease the interval
	Owner->SetActorTickInterval(UGameStateDataAsset::Get().GetTickInterval());
	Owner->SetActorTickEnabled(false);

#if WITH_EDITOR
	// Make this gameplay actor always loaded
	Owner->SetIsSpatiallyLoaded(false);
#endif

	// Set the movable mobility for in-game attaching
	if (Owner->GetRootComponent())
	{
		Owner->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

	// Finding the actor data asset
	ActorDataAssetInternal = UDataAssetsContainer::GetDataAssetByActorClass(Owner->GetClass());
	if (!ensureMsgf(ActorDataAssetInternal, TEXT("ASSERT: 'The Actor Data Asset' was not found")))
	{
		return;
	}

	// Initialize the Box Collision Component
	if (ensureMsgf(BoxCollisionComponentInternal, TEXT("ASSERT: 'BoxCollisionInternal' is not valid")))
	{
		BoxCollisionComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		BoxCollisionComponentInternal->SetBoxExtent(ActorDataAssetInternal->GetCollisionExtent());
#if WITH_EDITOR
		BoxCollisionComponentInternal->SetHiddenInGame(!bShouldShowRenders);
#endif
	}

	// Initialize mesh component
	if (ActorDataAssetInternal->GetActorType() == EAT::Player)
	{
		// The character class already has own initialized skeletal component
		MeshComponentInternal = Owner->FindComponentByClass<UMeshComponent>();
	}
	else
	{
		MeshComponentInternal = NewObject<UStaticMeshComponent>(Owner);
		MeshComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponentInternal->RegisterComponent();
	}
	checkf(MeshComponentInternal, TEXT("ERROR: [%i] %hs:\n'MeshComponentInternal' is null!"), __LINE__, __FUNCTION__);

	// Do not receive decals for level actors by default
	MeshComponentInternal->SetReceivesDecals(false);

	// On client, first spawn skips the SetActorHiddenInGame event in playing world on which level actors rely, call it manually
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !Owner->HasAuthority())
	{
		Owner->SetActorHiddenInGame(Owner->IsHidden());
	}

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
#if WITH_EDITORONLY_DATA
		// Should not call OnConstruction on drag events
		Owner->bRunConstructionScriptOnDrag = false;
#endif
	}
}

// Called when a component is destroyed for removing the owner from the Generated Map.
void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	AActor* ComponentOwner = GetOwner();
	if (ComponentOwner && IsValid(this) // Could be called multiple times, make sure it is called once for valid object
	    && !GExitPurge)                 // Do not call on exit
	{
		// Disable collision for safety
		ComponentOwner->SetActorEnableCollision(false);

		// Delete spawned collision component
		BoxCollisionComponentInternal->DestroyComponent();

		if (UUtilsLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			const UGeneratedMapSubsystem* GeneratedMapSubsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem();
			AGeneratedMap* GeneratedMap = GeneratedMapSubsystem ? GeneratedMapSubsystem->GetGeneratedMap() : nullptr;
			if (GeneratedMap) // Can be invalid if remove the Generated Map or opening another map
			{
				GeneratedMap->DestroyLevelActor(this);
			}

#if WITH_EDITOR
			UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
		}
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// Returns properties that are replicated for the lifetime of the actor channel
void UMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CellInternal);
	DOREPLIFETIME(ThisClass, RowIndexInternal);
	DOREPLIFETIME(ThisClass, CollisionResponseInternal);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Is called on an owner actor construction, could be called multiple times
bool UMapComponent::OnConstructionOwnerActor_Implementation()
{
	AActor* Owner = GetOwner();
	if (IS_TRANSIENT(Owner))
	{
		return false;
	}

	// Check the object state in the Pool Manager
	UPoolManagerSubsystem& PoolManager = UPoolManagerSubsystem::Get();
	const EPoolObjectState PoolObjectState = PoolManager.GetPoolObjectState(Owner);
	if (PoolObjectState == EPoolObjectState::None)
	{
		// The owner actor is not in the pool
		// Most likely it is a dragged actor, since all generated actors are always taken from the pool
		// Add this object to the pool and continue construction
		FPoolObjectData ObjectData(Owner);
		ObjectData.bIsActive = true;
		PoolManager.RegisterObjectInPool(ObjectData);
	}
	else if (PoolObjectState == EPoolObjectState::Inactive)
	{
		// Do not reconstruct inactive object
		return false;
	}

	AGeneratedMap& GeneratedMap = AGeneratedMap::Get();

	// Find new Location at dragging and update-delegate
	GeneratedMap.SetNearestCell(this);

	if (CellInternal.IsInvalidCell())
	{
		return false;
	}

	// Owner updating
	GeneratedMap.AddToGrid(this);
	if (IS_TRANSIENT(Owner)) // Check again, dragged owner can be moved to the persistent
	{
		return false;
	}

	// Set the default mesh, any system can override it later by calling SetCustomMeshAsset(Mesh). 
	const ULevelActorRow* FoundRow = GetActorDataAssetChecked().GetRowByLevelType(UMyBlueprintFunctionLibrary::GetLevelType());
	ensureMsgf(FoundRow && FoundRow->Mesh, TEXT("ASSERT: [%i] %hs:\n'FoundRow' is not valid, can not set the default mesh!"), __LINE__, __FUNCTION__);
	SetMesh(FoundRow->Mesh);

	TryDisplayOwnedCell();

	const ECollisionResponse CollisionResponse = GetActorDataAssetChecked().GetCollisionResponse();
	SetCollisionResponses(CollisionResponse);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
#if WITH_EDITOR
		// Update AI renders after adding obj to map
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
	}

	return true;
}

// Is called directly from Generated Map to broadcast OnPreRemovedFromLevel delegate and performs own logic
void UMapComponent::OnPreRemoved_Implementation(UObject* DestroyCauser)
{
	if (OnPreRemovedFromLevel.IsBound())
	{
		OnPreRemovedFromLevel.Broadcast(this, DestroyCauser);
	}
}

// Is called directly from Generated Map to broadcast OnPostRemovedFromLevel delegate and performs own logic
void UMapComponent::OnPostRemoved_Implementation(UObject* DestroyCauser/* = nullptr*/)
{
	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);

	if (OnPostRemovedFromLevel.IsBound())
	{
		OnPostRemovedFromLevel.Broadcast(this, DestroyCauser);
	}

	// -- Clear and discard all runtime changes

	SetCollisionResponses(ECR_Ignore);

	if (UUtilsLibrary::IsEditor())
	{
		// Remove all text renders of the Owner
		UCellsUtilsLibrary::ClearDisplayedCells(GetOwner());
	}

	// On server, reset the data (it will be replicated to clients)
	if (Owner->HasAuthority())
	{
		SetMesh(nullptr);
		SetCell(FCell::InvalidCell);
	}
}

/*********************************************************************************************
 * Editor
 ********************************************************************************************* */

#if WITH_EDITOR
// Returns whether this component or its owner is an editor-only object or not
bool UMapComponent::IsEditorOnly() const
{
	if (Super::IsEditorOnly())
	{
		return true;
	}

	const AActor* Owner = GetOwner();
	return Owner && Owner->IsEditorOnly();
}

// Destroy editoronly actor for the editor -game before registering the component
bool UMapComponent::Modify(bool bAlwaysMarkDirty/* = true*/)
{
	AActor* Owner = GetOwner();
	if (Owner
	    && !UUtilsLibrary::IsEditor() // is editor macro but not is GEditor, so [-game]
	    && IsEditorOnly())            // was generated in the editor
	{
		Owner->Destroy();
		return false;
	}

	return Super::Modify(bAlwaysMarkDirty);
}
#endif //WITH_EDITOR