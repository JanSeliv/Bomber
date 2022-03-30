﻿// Copyright (c) Yevhenii Selivanov.

#include "Components/MapComponent.h"
//---
#include "GeneratedMap.h"
#include "Components/SkeletalMeshComponent.h"
#include "Globals/LevelActorDataAsset.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
#include "PoolManager.h"
//---
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

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

// Updates a owner's state. Should be called in the owner's OnConstruction event.
bool UMapComponent::OnConstruction()
{
	AActor* Owner = GetOwner();
	if (IS_TRANSIENT(Owner))
	{
		return false;
	}

	AGeneratedMap& LevelMap = AGeneratedMap::Get();

	const UPoolManager* PoolManager = LevelMap.GetPoolManager();
	if (PoolManager
	    && !PoolManager->IsActive(Owner))
	{
		return false;
	}

	// Find new Location at dragging and update-delegate
	LevelMap.SetNearestCell(this);

	if (CellInternal.IsZeroCell())
	{
		return false;
	}

	// Owner updating
	LevelMap.AddToGrid(this);
	if (!IS_VALID(Owner)) // Dragged owner can be moved to the persistent
	{
		return false;
	}

	// Update default mesh asset
	if (ActorDataAssetInternal)
	{
		const ULevelActorRow* FoundRow = ActorDataAssetInternal->GetRowByLevelType(USingletonLibrary::GetLevelType());
		SetLevelActorRow(FoundRow);

		const ECollisionResponse CollisionResponse = ActorDataAssetInternal->GetCollisionResponse();
		SetCollisionResponses(CollisionResponse);
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Remove all text renders of the Owner
		USingletonLibrary::ClearOwnerTextRenders(Owner);

		// Update AI renders after adding obj to map
		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

		// Show current cell if type specified
		if (TO_FLAG(GetActorType()) & LevelMap.RenderActorsTypes)
		{
			USingletonLibrary::AddDebugTextRenders(Owner, {CellInternal}, FColor::White);
		}
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	return true;
}

// Override current cell data, where owner is located on the Level Map
void UMapComponent::SetCell(const FCell& Cell)
{
	CellInternal = Cell;
}

// Set specified mesh to the Owner
void UMapComponent::SetLevelActorRow(const ULevelActorRow* Row)
{
	if (!Row)
	{
		return;
	}

	LevelActorRowInternal = Row;

	// Update mesh
	UStreamableRenderAsset* Mesh = Row->Mesh;
	if (Mesh
	    && MeshComponentInternal) // is invalid for characters since it's controller by themselves
	{
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponentInternal))
		{
			SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(Mesh));
		}
		else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponentInternal))
		{
			StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(Mesh));
		}
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

// Rerun owner's construction scripts. The temporary only editor owner will not be updated
void UMapComponent::RerunOwnerConstruction() const
{
	AActor* Owner = GetOwner();
	if (ensureMsgf(IS_VALID(Owner), TEXT("RerunOwnerConstruction: The map owner is not valid")))
	{
		Owner->RerunConstructionScripts();
	}
}

// Get the owner's data asset
EActorType UMapComponent::GetActorType() const
{
	return ActorDataAssetInternal ? ActorDataAssetInternal->GetActorType() : EAT::None;
}

// Set true to make an owner to be undestroyable on this level
void UMapComponent::SetUndestroyable(bool bIsUndestroyable)
{
	bIsUndestroyableInternal = bIsUndestroyable;
}

// Set new collisions data for any channel of the Box Collision Component
void UMapComponent::SetCollisionResponses(const FCollisionResponseContainer& NewResponses)
{
	const AActor* Owner = GetOwner();
	if (!IS_VALID(Owner)
	    || !Owner->HasAuthority()
	    || NewResponses == ECR_MAX)
	{
		return;
	}

	CollisionResponseInternal = NewResponses;
	ApplyCollisionResponse();
}

// Is called when an owner was destroyed on the Level Map
void UMapComponent::OnDeactivated()
{
	if (OnDeactivatedMapComponent.IsBound())
	{
		OnDeactivatedMapComponent.Broadcast(this);
	}

	SetCollisionResponses(ECR_Ignore);

	if (IsUndestroyable())
	{
		SetUndestroyable(false);
	}
}

//  Called when a component is registered (not loaded
void UMapComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	if (!Owner
	    || ActorDataAssetInternal) // is already registered
	{
		return;
	}

	// Set the tick disabled by default and decrease the interval
	Owner->SetActorTickInterval(UGeneratedMapDataAsset::Get().GetTickInterval());
	Owner->SetActorTickEnabled(false);

	// Set the movable mobility for in-game attaching
	if (Owner->GetRootComponent())
	{
		Owner->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

	// Finding the actor data asset
	ActorDataAssetInternal = USingletonLibrary::GetDataAssetByActorClass(Owner->GetClass());
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
	if (ActorDataAssetInternal->GetActorType() != EAT::Player) // the character class already has own initialized skeletal component
	{
		MeshComponentInternal = NewObject<UMeshComponent>(GetOwner(), UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
		if (!ensureMsgf(MeshComponentInternal, TEXT("ASSERT: 'MeshComponentInternal' was not initialized")))
		{
			return;
		}
		MeshComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponentInternal->RegisterComponent();

		// Set default mesh asset
		const ULevelActorRow* FoundRow = ActorDataAssetInternal->GetRowByLevelType(USingletonLibrary::GetLevelType());
		SetLevelActorRow(FoundRow);
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Should not call OnConstruction on drag events
		Owner->bRunConstructionScriptOnDrag = false;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when a component is destroyed for removing the owner from the Level Map.
void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	AActor* ComponentOwner = GetOwner();
	if (!IS_TRANSIENT(ComponentOwner)) // Is not transient owner
	{
		// Disable collision for safety
		ComponentOwner->SetActorEnableCollision(false);

		// Delete spawned collision component
		BoxCollisionComponentInternal->DestroyComponent();

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			if (AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap()) // Can be invalid if remove the level map
			{
				LevelMap->DestroyLevelActor(this);
			}

			// Editor delegates
			USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
		}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// Returns properties that are replicated for the lifetime of the actor channel
void UMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CellInternal);
	DOREPLIFETIME(ThisClass, LevelActorRowInternal);
	DOREPLIFETIME(ThisClass, CollisionResponseInternal);
}

// Is called on client to update current level actor row
void UMapComponent::OnRep_LevelActorRow()
{
	SetLevelActorRow(LevelActorRowInternal);
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
	    && !USingletonLibrary::IsEditor() // is editor macro but not is GEditor, so [-game]
	    && IsEditorOnly())                // was generated in the editor
	{
		Owner->Destroy();
		return false;
	}

	return Super::Modify(bAlwaysMarkDirty);
}
#endif //WITH_EDITOR
