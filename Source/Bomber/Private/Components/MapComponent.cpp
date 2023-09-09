// Copyright (c) Yevhenii Selivanov.

#include "Components/MapComponent.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "PoolManagerSubsystem.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/GameStateDataAsset.h"
#include "DataAssets/LevelActorDataAsset.h"
#include "LevelActors/PlayerCharacter.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Components/BoxComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(MapComponent)

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

// Is called on an owner actor construction, could be called multiple times
bool UMapComponent::OnConstructionOwnerActor()
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
		PoolManager.RegisterObjectInPool(Owner, EPoolObjectState::Active);
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

	// Update default mesh asset
	const ULevelActorRow* FoundRow = GetActorDataAssetChecked().GetRowByLevelType(UMyBlueprintFunctionLibrary::GetLevelType());
	SetLevelActorRow(FoundRow);

	const ECollisionResponse CollisionResponse = GetActorDataAssetChecked().GetCollisionResponse();
	SetCollisionResponses(CollisionResponse);

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Update AI renders after adding obj to map
		UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	return true;
}

// Override current cell data, where owner is located on the Generated Map
void UMapComponent::SetCell(const FCell& Cell)
{
	CellInternal = Cell;

	TryDisplayOwnedCell();
}

// Show current cell if owned actor type is allowed, is not available in shipping build
void UMapComponent::TryDisplayOwnedCell()
{
#if !UE_BUILD_SHIPPING
	if (UCellsUtilsLibrary::CanDisplayCellsForActorTypes(TO_FLAG(GetActorType())))
	{
		FDisplayCellsParams Params = FDisplayCellsParams::EmptyParams;
		Params.bClearPreviousDisplays = true;
		UCellsUtilsLibrary::DisplayCell(GetOwner(), CellInternal, Params);
	}
#endif // !UE_BUILD_SHIPPING
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
	    && GetActorDataAssetChecked().GetActorType() != EActorType::Player) // characters are managing their mesh by themselves
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

// Returns the map component of the specified owner
UMapComponent* UMapComponent::GetMapComponent(const AActor* Owner)
{
	return Owner ? Owner->FindComponentByClass<UMapComponent>() : nullptr;
}

// Get the owner's data asset
EActorType UMapComponent::GetActorType() const
{
	return GetActorDataAssetChecked().GetActorType();
}

// Get the owner's data asset
const ULevelActorDataAsset& UMapComponent::GetActorDataAssetChecked() const
{
	checkf(ActorDataAssetInternal, TEXT("EROR: 'ActorDataAssetInternal' is null"));
	return *ActorDataAssetInternal;
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

// Is called when an owner was destroyed on the Generated Map
void UMapComponent::OnDeactivated(UObject* DestroyCauser/* = nullptr*/)
{
	if (OnDeactivatedMapComponent.IsBound())
	{
		OnDeactivatedMapComponent.Broadcast(this, DestroyCauser);
	}

	SetCollisionResponses(ECR_Ignore);

	if (IsUndestroyable())
	{
		SetUndestroyable(false);
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditor())
	{
		// Remove all text renders of the Owner
		UCellsUtilsLibrary::ClearDisplayedCells(GetOwner());
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

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
		const ACharacter* Player = CastChecked<ACharacter>(GetOwner());
		MeshComponentInternal = Player->GetMesh();
		check(MeshComponentInternal);
	}
	else
	{
		MeshComponentInternal = NewObject<UMeshComponent>(GetOwner(), UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
		MeshComponentInternal->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponentInternal->RegisterComponent();

		// Set default mesh asset
		const ULevelActorRow* FoundRow = ActorDataAssetInternal->GetRowByLevelType(UMyBlueprintFunctionLibrary::GetLevelType());
		SetLevelActorRow(FoundRow);
	}

	// Do not receive decals for level actors by default
	MeshComponentInternal->SetReceivesDecals(false);

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		// Should not call OnConstruction on drag events
		Owner->bRunConstructionScriptOnDrag = false;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
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

#if WITH_EDITOR	// [IsEditorNotPieWorld]
		if (FEditorUtilsLibrary::IsEditorNotPieWorld())
		{
			// The owner was removed from the editor level
			const UGeneratedMapSubsystem* GeneratedMapSubsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem();
			AGeneratedMap* GeneratedMap = GeneratedMapSubsystem ? GeneratedMapSubsystem->GetGeneratedMap() : nullptr;
			if (GeneratedMap) // Can be invalid if remove the Generated Map or opening another map
			{
				GeneratedMap->DestroyLevelActor(this);
			}

			// Editor delegates
			UMyUnrealEdEngine::GOnAIUpdatedDelegate.Broadcast();
		}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
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
	    && !FEditorUtilsLibrary::IsEditor() // is editor macro but not is GEditor, so [-game]
	    && IsEditorOnly())                  // was generated in the editor
	{
		Owner->Destroy();
		return false;
	}

	return Super::Modify(bAlwaysMarkDirty);
}
#endif //WITH_EDITOR
