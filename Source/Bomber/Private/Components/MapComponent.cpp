﻿// Copyright 2020 Yevhenii Selivanov.

#include "Components/MapComponent.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "Globals/SingletonLibrary.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "Components/BoxComponent.h"

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Initialize the Box Collision component
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollisionComponent");
}

// Updates a owner's state. Should be called in the owner's OnConstruction event.
void UMapComponent::OnConstruction()
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	AActor* Owner = GetOwner();
	if (!IS_VALID(Owner) // The owner is not valid
	    || !LevelMap)    // The Level Map is not valid
	{
		return;
	}

	// Find new Location at dragging and update-delegate
	USingletonLibrary::PrintToLog(Owner, "OnMapComponentConstruction", "-> \t FCell()");
	LevelMap->SetNearestCell(this);

	// Owner updating
	USingletonLibrary::PrintToLog(Owner, "OnMapComponentConstruction", "-> \t AddToGrid");
	USingletonLibrary::GetLevelMap()->AddToGrid(Cell, this);

	// Set default mesh asset
	if (ActorDataAssetInternal)
	{
		const ULevelActorRow* FoundRow = ActorDataAssetInternal->GetRowByLevelType(USingletonLibrary::GetLevelType());
		SetMeshByRow(FoundRow);
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Remove all text renders of the Owner
		USingletonLibrary::PrintToLog(Owner, "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(Owner);

		// Update AI renders after adding obj to map
		USingletonLibrary::PrintToLog(Owner, "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t BroadcastAiUpdating");
		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Set specified mesh to the Owner
void UMapComponent::SetMeshByRow(const ULevelActorRow* Row, UMeshComponent* InMeshComponent/* = nullptr*/)
{
	if (!Row
	    || !MeshComponentInternal && !InMeshComponent)
	{
		return;
	}

	if (InMeshComponent)
	{
		MeshComponentInternal = InMeshComponent;
	}

	if (auto SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponentInternal))
	{
		SkeletalMeshComponent->SetSkeletalMesh(Cast<USkeletalMesh>(Row->Mesh));
	}
	else if (auto StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponentInternal))
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
	if (!ensureMsgf(GetOwner(), TEXT("RerunOwnerConstruction: The map owner is not valid"))) return;
	GetOwner()->RerunConstructionScripts();
}

// Get the owner's data asset
EActorType UMapComponent::GetActorType() const
{
	return ActorDataAssetInternal ? ActorDataAssetInternal->GetActorType() : EAT::None;
}

//  Called when a component is registered (not loaded
void UMapComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	if (!IS_VALID(Owner)) // owner is not valid
	{
		return;
	}
	USingletonLibrary::PrintToLog(Owner, "OnRegister", "");

	// Disable the tick
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
	if (IS_TRANSIENT(ComponentOwner) == false         // Is not transient owner
	    && IsValid(USingletonLibrary::GetLevelMap())) // is valid and is not transient the level map
	{
		USingletonLibrary::PrintToLog(ComponentOwner, "OnComponentDestroyed", "-> \t DestroyActorsFromMap");

		//disable collision for safety
		ComponentOwner->SetActorEnableCollision(false);

		// Delete spawned collision component
		BoxCollision->DestroyComponent();

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			USingletonLibrary::GetLevelMap()->DestroyLevelActor(this);

			// Editor delegates
			USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
		}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
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
