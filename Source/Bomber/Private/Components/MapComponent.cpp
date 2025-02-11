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
}

/*********************************************************************************************
 * Cell (Location)
 ********************************************************************************************* */

// Allows to change locally the cell of the owner on the Generated Map
void UMapComponent::SetCell(const FCell& Cell)
{
	if (Cell == LocalCellInternal)
	{
		return;
	}

	const FCell PreviousCell = LocalCellInternal;

	// Set new cell locally, is not replicated here, but in the Map Components Container which is changed by the Generated Map
	LocalCellInternal = Cell;

	TryDisplayOwnedCell();

	if (OnCellChanged.IsBound())
	{
		OnCellChanged.Broadcast(this, LocalCellInternal, PreviousCell);
	}
}

// Show current cell if owned actor type is allowed, is not available in shipping build
void UMapComponent::TryDisplayOwnedCell(bool bClearPrevious/* = false*/)
{
#if !UE_BUILD_SHIPPING
	FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
	Params.bClearPreviousDisplays = bClearPrevious
	                                || UUtilsLibrary::IsEditorNotPieWorld(); // Always clear before PIE, so it properly updates when uncheck bShouldShowRenders
	UCellsUtilsLibrary::DisplayCell(GetOwner(), LocalCellInternal, Params);
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
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RowIndexInternal, this);

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

// Returns current collisions data of the Box Collision Component
FCollisionResponseContainer UMapComponent::GetCollisionResponses() const
{
	return BoxCollisionComponentInternal ? BoxCollisionComponentInternal->GetCollisionResponseToChannels() : FCollisionResponseContainer(ECR_MAX);
}

// Set new collisions data for any channel of the Box Collision Component
void UMapComponent::SetCollisionResponses(const FCollisionResponseContainer& NewResponses)
{
	const AActor* Owner = GetOwner();
	if (!Owner
	    || !GetActorDataAssetChecked().IsEnabledCollision()
	    || NewResponses == ECR_MAX
	    || NewResponses == GetCollisionResponses())
	{
		return;
	}

	checkf(BoxCollisionComponentInternal, TEXT("ERROR: [%i] %hs:\n'BoxCollisionComponentInternal' is null!"), __LINE__, __FUNCTION__);
	BoxCollisionComponentInternal->SetCollisionResponseToChannels(NewResponses);
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
	USceneComponent* OwnerRootComponent = Owner->GetRootComponent();
	checkf(OwnerRootComponent, TEXT("ERROR: [%i] %hs:\n'OwnerRootComponent' is null!"), __LINE__, __FUNCTION__);
	OwnerRootComponent->SetMobility(EComponentMobility::Movable);

	// Finding the actor data asset
	ActorDataAssetInternal = UDataAssetsContainer::GetDataAssetByActorClass(Owner->GetClass());
	if (!ensureMsgf(ActorDataAssetInternal, TEXT("ASSERT: 'The Actor Data Asset' was not found")))
	{
		return;
	}

	// Initialize the Box Collision Component
	if (ActorDataAssetInternal->IsEnabledCollision())
	{
		BoxCollisionComponentInternal = NewObject<UBoxComponent>(Owner);
		BoxCollisionComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		BoxCollisionComponentInternal->SetBoxExtent(ActorDataAssetInternal->GetCollisionExtent());
		BoxCollisionComponentInternal->IgnoreActorWhenMoving(Owner, true);
#if WITH_EDITOR
		BoxCollisionComponentInternal->SetHiddenInGame(!bShouldShowRenders);
#endif
		BoxCollisionComponentInternal->RegisterComponent();
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
	const AActor* ComponentOwner = GetOwner();
	if (ComponentOwner && IsValid(this) // Could be called multiple times, make sure it is called once for valid object
		&& !GExitPurge)                 // Do not call on exit
	{
		if (UUtilsLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			if (AGeneratedMap* GeneratedMap = AGeneratedMap::GetGeneratedMap()) // Can be invalid if remove the Generated Map or opening another map
			{
				GeneratedMap->DestroyLevelActor(this);
			}

#if WITH_EDITOR
			UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
		}

		// Delete spawned collision component
		if (IsValid(BoxCollisionComponentInternal))
		{
			BoxCollisionComponentInternal->DestroyComponent();
			BoxCollisionComponentInternal = nullptr;
		}
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// Returns properties that are replicated for the lifetime of the actor channel
void UMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RowIndexInternal, Params);
}

/*********************************************************************************************
 * Events
 ********************************************************************************************* */

// Called when this level actor is reconstructed or added on the Generated Map
bool UMapComponent::OnAdded_Implementation()
{
	AActor* Owner = GetOwner();
	checkf(Owner, TEXT("ERROR: [%i] %hs:\n'Owner' is null!"), __LINE__, __FUNCTION__);

	TRACE_CPUPROFILER_EVENT_SCOPE(UMapComponent::OnAdded);

	// Set the default mesh, any system can override it later by calling SetCustomMeshAsset(Mesh). 
	const ULevelActorRow* FoundRow = GetActorDataAssetChecked().GetRowByLevelType(UMyBlueprintFunctionLibrary::GetLevelType());
	ensureMsgf(FoundRow && FoundRow->Mesh, TEXT("ASSERT: [%i] %hs:\n'FoundRow' is not valid, can not set the default mesh!"), __LINE__, __FUNCTION__);
	SetMesh(FoundRow->Mesh);

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
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
#endif
	}

	// Notify listeners about the actor was added to the level
	OnAddedToLevel.Broadcast(this);

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

	SetMesh(nullptr);

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