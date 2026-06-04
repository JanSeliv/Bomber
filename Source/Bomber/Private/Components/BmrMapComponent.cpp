// Copyright (c) Yevhenii Selivanov.

#include "Components/BmrMapComponent.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "Bomber.h"
#include "DalRegistrySubsystem.h"
#include "DataAssets/BmrGameStateDataAsset.h"
#include "DataAssets/BmrLevelActorDataAsset.h"
#include "DataRegistries/BmrLevelActorRow.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#endif

// UE
#include "Components/BoxComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/StreamableRenderAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrMapComponent)

// Returns the map component of the specified owner
UBmrMapComponent* UBmrMapComponent::GetMapComponent(const AActor* Owner)
{
	return Owner ? Owner->FindComponentByClass<UBmrMapComponent>() : nullptr;
}

// Sets default values for this component's properties
UBmrMapComponent::UBmrMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

/*********************************************************************************************
 * Cell (Location)
 ********************************************************************************************* */

// Allows to change locally the cell of the owner on the Generated Map
void UBmrMapComponent::SetCell(const FBmrCell& Cell)
{
	if (Cell == LocalCell)
	{
		return;
	}

	const FBmrCell PreviousCell = LocalCell;

	// Set new cell locally, is not replicated here, but in the Map Components Container which is changed by the Generated Map
	LocalCell = Cell;

	TryDisplayOwnedCell();

	if (OnCellChanged.IsBound())
	{
		OnCellChanged.Broadcast(this, LocalCell, PreviousCell);
	}
}

// Show current cell if owned actor type is allowed, is not available in shipping build
void UBmrMapComponent::TryDisplayOwnedCell(bool bClearPrevious /* = false*/)
{
#if !UE_BUILD_SHIPPING
	FBmrDisplayCellsParams Params = FBmrDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = bClearPrevious
	                                || UUtilsLibrary::IsEditorNotPieWorld(); // Always clear before PIE, so it properly updates when uncheck bShouldShowRenders
	UBmrCellUtilsLibrary::DisplayCell(GetOwner(), LocalCell, Params);
#endif // !UE_BUILD_SHIPPING
}

/*********************************************************************************************
 * Mesh
 ********************************************************************************************* */

// Sets the mesh component of the owner actor
void UBmrMapComponent::SetMeshComponent(UMeshComponent* NewMeshComponent)
{
	if (!ensureMsgf(NewMeshComponent, TEXT("ASSERT: [%i] %hs:\n'NewMeshComponent' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	if (IsValid(MeshComponent))
	{
		MeshComponent->DestroyComponent();
	}

	MeshComponent = NewMeshComponent;
}

// Returns current mesh asset
class UStreamableRenderAsset* UBmrMapComponent::GetMesh() const
{
	return UGameplayUtilsLibrary::GetMesh(MeshComponent);
}

// Applies given mesh on owner actor, or resets the mesh if null is passed
void UBmrMapComponent::SetLocalMesh(UStreamableRenderAsset* NewMesh)
{
	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);

	UGameplayUtilsLibrary::SetMesh(MeshComponent, NewMesh);
}

/** Set material to the mesh. */
void UBmrMapComponent::SetLocalMeshMaterial(UMaterialInterface* NewMaterial)
{
	if (!ensureMsgf(NewMaterial, TEXT("ASSERT: [%i] %hs:\n'Material' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(MeshComponent, TEXT("ASSERT: [%i] %hs:\n'MeshComponent' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	MeshComponent->SetMaterial(0, NewMaterial);
}

// Resolves the Data Registry row by name and applies its mesh locally
void UBmrMapComponent::TryApplyMeshFromRow(FName RowName)
{
	const UScriptStruct* RowType = GetActorDataAssetChecked().GetRowType();
	if (!ensureMsgf(RowType, TEXT("ASSERT: [%i] %hs:\n'RowType' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Row or Mesh might be not loaded yet: try apply now or fallback for listening
	const FBmrLevelActorRow* Row = RowName.IsNone() ? FBmrLevelActorRow::FindFirstRow(RowType) : FBmrLevelActorRow::FindRowByName(RowType, RowName);
	UStreamableRenderAsset* ResolvedMesh = Row ? Row->Mesh.Get() : nullptr;

	if (ResolvedMesh)
	{
		SetLocalMesh(ResolvedMesh);
		return;
	}

	// Fallback to listen for the row to be loaded or updated in the Data Registry, and apply the mesh then
	// None row name waits for the first/any row of this type as default before specific row resolves
	UDalRegistrySubsystem::Get().ListenForDataRegistryRow(this, RowType, RowName, [this](FName ResolvedRowName)
	{
		const UScriptStruct* ResolvedRowType = GetActorDataAssetChecked().GetRowType();
		const FBmrLevelActorRow* ResolvedRow = FBmrLevelActorRow::FindRowByName(ResolvedRowType, ResolvedRowName);
		UStreamableRenderAsset* Mesh = ResolvedRow ? ResolvedRow->Mesh.Get() : nullptr;
		ensureMsgf(Mesh, TEXT("ASSERT: [%i] %hs:\n'Mesh' is not valid for row '%s'!"), __LINE__, __FUNCTION__, *ResolvedRowName.ToString());
		SetLocalMesh(Mesh);
	});
}

/*********************************************************************************************
 * Collision
 ********************************************************************************************* */

// Returns current collisions data of the Box Collision Component
FCollisionResponseContainer UBmrMapComponent::GetCollisionResponses() const
{
	return BoxCollisionComponent ? BoxCollisionComponent->GetCollisionResponseToChannels() : FCollisionResponseContainer(ECR_MAX);
}

// Set new collisions data for any channel of the Box Collision Component
void UBmrMapComponent::SetCollisionResponses(const FCollisionResponseContainer& NewResponses)
{
	const AActor* Owner = GetOwner();
	if (!Owner
	    || !GetActorDataAssetChecked().IsEnabledCollision()
	    || NewResponses == ECR_MAX
	    || NewResponses == GetCollisionResponses())
	{
		return;
	}

	checkf(BoxCollisionComponent, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponent' is null!"), __LINE__, __FUNCTION__);
	BoxCollisionComponent->SetCollisionResponseToChannels(NewResponses);
}

/*********************************************************************************************
 * Data Asset
 ********************************************************************************************* */

// Get the owner's data asset
const UBmrLevelActorDataAsset& UBmrMapComponent::GetActorDataAssetChecked() const
{
	checkf(ActorDataAsset, TEXT("EROR: 'ActorDataAsset' is null"));
	return *ActorDataAsset;
}

// Get the owner's data asset
EBmrActorType UBmrMapComponent::GetActorType() const
{
	return ActorDataAsset ? ActorDataAsset->GetActorType() : EBmrActorType::None;
}

/*********************************************************************************************
 * Overrides
 ********************************************************************************************* */

//  Called when a component is registered (not loaded)
void UBmrMapComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	check(Owner);

	// Register level actors for game features only if the game was started
	constexpr bool bAddOnlyInGameWorlds = true;
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(Owner, bAddOnlyInGameWorlds);

	if (ActorDataAsset)
	{
		// Its data asset is valid, so initialization was already performed before
		return;
	}

	// Set the tick disabled by default and decrease the interval
	Owner->SetActorTickInterval(UBmrGameStateDataAsset::GTickInterval);
	Owner->SetActorTickEnabled(false);

#if WITH_EDITOR
	// Make this gameplay actor always loaded
	Owner->SetIsSpatiallyLoaded(false);
#endif

	// Set the movable mobility for in-game attaching
	USceneComponent* OwnerRootComponent = Owner->GetRootComponent();
	checkf(OwnerRootComponent, TEXT("ERROR: [%i] %hs:\n'OwnerRootComponent' is null!"), __LINE__, __FUNCTION__);
	OwnerRootComponent->SetMobility(EComponentMobility::Movable);

	// Finding the actor data asset
	ActorDataAsset = UBmrBlueprintFunctionLibrary::GetDataAssetByLevelActor(Owner);
	if (!ensureMsgf(ActorDataAsset, TEXT("ASSERT: 'The Actor Data Asset' was not found for '%s' actor!"), *GetNameSafe(Owner)))
	{
		return;
	}

	// Initialize the Box Collision Component
	if (ActorDataAsset->IsEnabledCollision())
	{
		BoxCollisionComponent = NewObject<UBoxComponent>(Owner);
		BoxCollisionComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		BoxCollisionComponent->SetBoxExtent(ActorDataAsset->GetCollisionExtent());
		BoxCollisionComponent->IgnoreActorWhenMoving(Owner, true);
		SetCollisionResponses(ActorDataAsset->GetCollisionResponse());
#if WITH_EDITOR
		BoxCollisionComponent->SetHiddenInGame(!bShouldShowRenders);
#endif
		BoxCollisionComponent->RegisterComponent();
	}

	// Initialize mesh component
	if (!MeshComponent)
	{
		MeshComponent = NewObject<UStaticMeshComponent>(Owner);
		MeshComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponent->RegisterComponent();
	}
	checkf(MeshComponent, TEXT("ERROR: [%i] %hs:\n'MeshComponent' is null!"), __LINE__, __FUNCTION__);

	// Do not receive decals for level actors by default
	MeshComponent->SetReceivesDecals(false);

	// On client, first spawn skips the SetActorHiddenInGame event in playing world on which level actors rely, call it manually
	if (UUtilsLibrary::HasWorldBegunPlay()
	    && !Owner->HasAuthority())
	{
		Owner->SetActorHiddenInGame(Owner->IsHidden());
	}

	if (ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap())
	{
		// Manually resolve replicated Map Component if spawned late
		GeneratedMap->ResolveSpawnedMapComponent(*this);
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
void UBmrMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	const AActor* ComponentOwner = GetOwner();
	if (ComponentOwner && IsValid(this) // Could be called multiple times, make sure it is called once for valid object
	    && !GExitPurge) // Do not call on exit
	{
		if (UUtilsLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			if (ABmrGeneratedMap* GeneratedMap = ABmrGeneratedMap::GetGeneratedMap()) // Can be invalid if remove the Generated Map or opening another map
			{
				GeneratedMap->DestroyLevelActor(this);
			}

#if WITH_EDITOR
			UBmrUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
		}

		// Delete spawned collision component
		if (IsValid(BoxCollisionComponent))
		{
			BoxCollisionComponent->DestroyComponent();
			BoxCollisionComponent = nullptr;
		}
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
bool UBmrMapComponent::OnAdded_Implementation()
{
	AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);

	TRACE_CPUPROFILER_EVENT_SCOPE(UMapComponent::OnAdded);

	Activate();

	// Set the default mesh (if any custom or replicated is not set yet), any system can override it later
	if (!GetMesh())
	{
		TryApplyMeshFromRow(NAME_None);
	}

	TryDisplayOwnedCell();

	// Apply default collision
	const ECollisionResponse CollisionResponse = GetActorDataAssetChecked().GetCollisionResponse();
	SetCollisionResponses(CollisionResponse);

	// Disable tick by default: actor itself might re-enable it in runtime like from game state change
	Owner->SetActorTickEnabled(false);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
#if WITH_EDITOR
		// Update AI renders after adding obj to map
		UBmrUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
	}

	// Notify listeners about the actor was added to the level
	OnAddedToLevel.Broadcast(this);
	BPOnAddedToLevel.Broadcast(this);

	// Increment the token to track the replication changes
	ABmrGeneratedMap::Get().IncrementReplicationToken();

	return true;
}

// Is called directly from Generated Map to broadcast OnPreRemovedFromLevel delegate and performs own logic
void UBmrMapComponent::OnPreRemoved_Implementation(UObject* DestroyCauser)
{
	if (OnPreRemovedFromLevel.IsBound())
	{
		OnPreRemovedFromLevel.Broadcast(this, DestroyCauser);
	}

	// Mark this component as unregistered (while OnPostRemoved means it is fully removed)
	Deactivate();
}

// Is called directly from Generated Map to broadcast OnPostRemovedFromLevel delegate and performs own logic
void UBmrMapComponent::OnPostRemoved_Implementation(UObject* DestroyCauser /* = nullptr*/)
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
		UBmrCellUtilsLibrary::ClearDisplayedCells(GetOwner());
	}

	SetLocalMesh(nullptr);

	SetCell(FBmrCell::InvalidCell);
}

/*********************************************************************************************
 * Editor
 ********************************************************************************************* */

#if WITH_EDITOR
// Returns whether this component or its owner is an editor-only object or not
bool UBmrMapComponent::IsEditorOnly() const
{
	if (Super::IsEditorOnly())
	{
		return true;
	}

	const AActor* Owner = GetOwner();
	return Owner && Owner->IsEditorOnly();
}

// Destroy editoronly actor for the editor -game before registering the component
bool UBmrMapComponent::Modify(bool bAlwaysMarkDirty /* = true*/)
{
	AActor* Owner = GetOwner();
	if (Owner
	    && !UUtilsLibrary::IsEditor() // is editor macro but not is GEditor, so [-game]
	    && IsEditorOnly()) // was generated in the editor
	{
		Owner->Destroy();
		return false;
	}

	return Super::Modify(bAlwaysMarkDirty);
}
#endif // WITH_EDITOR