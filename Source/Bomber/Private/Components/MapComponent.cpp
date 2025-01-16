﻿// Copyright (c) Yevhenii Selivanov.

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
	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);
	if (!Owner->HasAuthority())
	{
		return;
	}

	CellInternal = Cell;

	TryDisplayOwnedCell();
}

// Show current cell if owned actor type is allowed, is not available in shipping build
void UMapComponent::TryDisplayOwnedCell()
{
#if !UE_BUILD_SHIPPING
	FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = true;
	UCellsUtilsLibrary::DisplayCell(GetOwner(), CellInternal, Params);
#endif // !UE_BUILD_SHIPPING
}

// Is called on client to update current cell
void UMapComponent::OnRep_Cell()
{
	if (CellInternal.IsInvalidCell())
	{
		// It's client which invalidated the cell, broadcast events manually as initial removal happened only on the server
		OnDeactivated();
	}
}

/*********************************************************************************************
 * Mesh
 ********************************************************************************************* */

// Updates current mesh to default according current level type
void UMapComponent::SetDefaultMesh()
{
	if (GetActorDataAssetChecked().GetActorType() == EActorType::Player)
	{
		// ACharacter has own mesh component, no need to manage it
		return;
	}

	const ULevelActorRow* FoundRow = GetActorDataAssetChecked().GetRowByLevelType(UMyBlueprintFunctionLibrary::GetLevelType());
	if (!ensureMsgf(FoundRow, TEXT("ASSERT: [%i] %hs:\n'FoundRow' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	SetCustomMeshAsset(FoundRow->Mesh);
}

// Returns mesh asset if changed or null if default
class UStreamableRenderAsset* UMapComponent::GetCustomMeshAsset() const
{
	if (UStreamableRenderAsset* CurrentMesh = UGameplayUtilsLibrary::GetMesh(MeshComponentInternal))
	{
		return CurrentMesh;
	}

	// Try to find the mesh by the row index if the mesh is not set yet for any reason
	if (RowIndexInternal != INDEX_NONE)
	{
		const ULevelActorRow* FoundRow = ActorDataAssetInternal ? ActorDataAssetInternal->GetRowByIndex(RowIndexInternal) : nullptr;
		return FoundRow ? FoundRow->Mesh : nullptr;
	}

	return nullptr;
}

// Overrides mesh to the Owner ignoring current level type and Level Row
void UMapComponent::SetCustomMeshAsset(UStreamableRenderAsset* CustomMeshAsset)
{
	if (!ensureMsgf(CustomMeshAsset, TEXT("ASSERT: [%i] %hs:\n'CustomMeshAsset' is null, attempted to set null mesh!"), __LINE__, __FUNCTION__)
	    || UGameplayUtilsLibrary::GetMesh(MeshComponentInternal) == CustomMeshAsset) // Is already set
	{
		return;
	}

	UGameplayUtilsLibrary::SetMesh(MeshComponentInternal, CustomMeshAsset);

	// Set the index of the mesh, is primarily used for replicating the mesh to the clients
	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);
	if (Owner->HasAuthority())
	{
		checkf(ActorDataAssetInternal, TEXT("ERROR: [%i] %hs:\n'ActorDataAssetInternal' is null!"), __LINE__, __FUNCTION__);
		const int32 NewRowIndex = ActorDataAssetInternal->GetIndexByRow(ActorDataAssetInternal->GetRowByMesh(CustomMeshAsset));
		ensureMsgf(NewRowIndex != INDEX_NONE, TEXT("ASSERT: [%i] %hs:\nAttempted to set the '%s' mesh on '%s' actor that is not stored in the '%s' data asset!"), __LINE__, __FUNCTION__, *GetNameSafe(CustomMeshAsset), *GetNameSafe(Owner), *GetNameSafe(ActorDataAssetInternal));
		RowIndexInternal = NewRowIndex;
	}
}

// Sets mesh to empty, is used for cleanup
void UMapComponent::ResetMesh()
{
	if (GetActorDataAssetChecked().GetActorType() == EActorType::Player)
	{
		// ACharacter has own mesh component, no need to manage it
		return;
	}

	UGameplayUtilsLibrary::SetMesh(MeshComponentInternal, nullptr);

	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);
	if (Owner->HasAuthority()
	    && RowIndexInternal != INDEX_NONE)
	{
		RowIndexInternal = INDEX_NONE;
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
void UMapComponent::OnRep_RowIndex()
{
	if (RowIndexInternal == INDEX_NONE)
	{
		// It was reset to default mesh on server
		ResetMesh();
		return;
	}

	// Apply replicated mesh on client
	const ULevelActorRow* ReplicatedRow = GetActorDataAssetChecked().GetRowByIndex(RowIndexInternal);
	if (ensureMsgf(ReplicatedRow, TEXT("ASSERT: [%i] %hs:\n'ReplicatedRow' is null!"), __LINE__, __FUNCTION__))
	{
		SetCustomMeshAsset(ReplicatedRow->Mesh);
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
		check(MeshComponentInternal);
	}
	else
	{
		MeshComponentInternal = NewObject<UStaticMeshComponent>(Owner);
		MeshComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponentInternal->RegisterComponent();
	}

	// Do not receive decals for level actors by default
	MeshComponentInternal->SetReceivesDecals(false);

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

	SetDefaultMesh();

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

// Is called directly from Generated Map to broadcast delegate and performs own logic
void UMapComponent::OnDeactivated_Implementation(UObject* DestroyCauser/* = nullptr*/)
{
	if (OnDeactivatedMapComponent.IsBound())
	{
		OnDeactivatedMapComponent.Broadcast(this, DestroyCauser);
	}

	// -- Clear and discard all runtime changes

	SetCollisionResponses(ECR_Ignore);

	ResetMesh();

	if (IsUndestroyable())
	{
		SetUndestroyable(false);
	}

	if (UUtilsLibrary::IsEditor())
	{
		// Remove all text renders of the Owner
		UCellsUtilsLibrary::ClearDisplayedCells(GetOwner());
	}

	SetCell(FCell::InvalidCell);
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