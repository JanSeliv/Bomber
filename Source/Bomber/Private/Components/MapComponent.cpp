﻿// Copyright 2021 Yevhenii Selivanov.

#include "Components/MapComponent.h"
//---
#include "GeneratedMap.h"
#include "Components/SkeletalMeshComponent.h"
#include "Globals/LevelActorDataAsset.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/PlayerCharacter.h"
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
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
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

	// Find new Location at dragging and update-delegate
	LevelMap.SetNearestCell(this);

	// Owner updating
	LevelMap.AddToGrid(Cell, this);
	if (!IS_VALID(Owner)) // Dragged owner can be moved to the persistent
	{
		return false;
	}

	// Update default mesh asset
	if (ActorDataAssetInternal)
	{
		const ULevelActorRow* FoundRow = ActorDataAssetInternal->GetRowByLevelType(USingletonLibrary::GetLevelType());
		SetMeshByRow(FoundRow);
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
			USingletonLibrary::AddDebugTextRenders(Owner, {Cell}, FColor::White);
		}
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	return true;
}

// Set specified mesh to the Owner
void UMapComponent::SetMeshByRow(const ULevelActorRow* Row, UMeshComponent* InMeshComponent/* = nullptr*/)
{
	if (!Row
	    || !Row->Mesh
	    || !MeshComponentInternal && !InMeshComponent)
	{
		return;
	}

	if (InMeshComponent)
	{
		MeshComponentInternal = InMeshComponent;
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponentInternal))
	{
		SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(Row->Mesh));
	}
	else if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponentInternal))
	{
		StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(Row->Mesh));
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
	if (ensureMsgf(BoxCollision, TEXT("ASSERT: 'BoxCollisionInternal' is not valid")))
	{
		BoxCollision->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		BoxCollision->SetBoxExtent(ActorDataAssetInternal->GetCollisionExtent());
		BoxCollision->SetCollisionResponseToAllChannels(ActorDataAssetInternal->GetCollisionResponse());
#if WITH_EDITOR
		BoxCollision->SetHiddenInGame(!bShouldShowRenders);
#endif
	}

	// Initialize mesh component
	if (ActorDataAssetInternal->GetActorType() != EActorType::Player) // the character class already has own initialized skeletal component
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
		SetMeshByRow(FoundRow);
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
	AActor* const ComponentOwner = GetOwner();
	if (!IS_TRANSIENT(ComponentOwner)) // Is not transient owner
	{
		// Disable collision for safety
		ComponentOwner->SetActorEnableCollision(false);

		// Delete spawned collision component
		BoxCollision->DestroyComponent();

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			AGeneratedMap::Get().DestroyLevelActor(this);

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

	DOREPLIFETIME(ThisClass, Cell);
}

#if WITH_EDITOR
// Returns whether this component or its owner is an editor-only object or not
bool UMapComponent::IsEditorOnly() const
{
	AActor* Owner = GetOwner();
	return Super::IsEditorOnly() || Owner && Owner->IsEditorOnly();
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
