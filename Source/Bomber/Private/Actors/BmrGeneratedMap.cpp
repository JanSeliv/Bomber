// Copyright (c) Yevhenii Selivanov.

#include "Actors/BmrGeneratedMap.h"

// Bomber
#include "AbilitySystem/Attributes/BmrHealthAttributeSet.h"
#include "Components/BmrCameraComponent.h"
#include "Components/BmrMapComponent.h"
#include "DalSubsystem.h"
#include "Data/SpawnRequest.h"
#include "DataAssets/BmrGeneratedMapDataAsset.h"
#include "Engine/Engine.h"
#include "Generators/BmrCellsGenerator_Base.h"
#include "MyUtilsLibraries/AsyncLoadUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "PoolManagerSubsystem.h"
#include "Structures/BmrGameStateTag.h"
#include "Structures/BmrGameplayTags.h"
#include "Subsystems/BmrGeneratedMapSubsystem.h"
#include "Subsystems/GlobalMessageSubsystem.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

#if WITH_EDITOR
#include "BmrUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif

// UE
#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrGeneratedMap)

// Empty generation settings instance
const FBmrGeneratedMapSettings FBmrGeneratedMapSettings::Empty = FBmrGeneratedMapSettings();

/* ---------------------------------------------------
 *		Generated Map public functions
 * --------------------------------------------------- */

// Sets default values
ABmrGeneratedMap::ABmrGeneratedMap()
{
	// Set this actor to call Tick() every time to update characters locations
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

#if WITH_EDITORONLY_DATA
	// Make this gameplay actor always loaded
	bIsSpatiallyLoaded = false;
#endif

	// Replicate an actor
	bReplicates = true;
	static constexpr float NewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewUpdateFrequency);
	bAlwaysRelevant = true;

#if WITH_EDITOR //[Editor]
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;
#endif // WITH_EDITOR [Editor]

	// Initialize the Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	static const FVector DefaultRelativeScale(9.F, 9.F, 1.F);
	RootComponent->SetRelativeScale3D_Direct(DefaultRelativeScale);
	RootComponent->SetIsReplicated(true); // Enable to replicate own transform and attached level actors

	// Find blueprint class of the background
	CollisionComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Collision Component"));
	CollisionComponent->SetupAttachment(RootComponent);

	// Default camera class
	CameraComponent = CreateDefaultSubobject<UBmrCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(RootComponent);

	// Create ASC on Generated Map for environmental abilities and level-related mods (e.g., environmental explosions)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	HealthSet = CreateDefaultSubobject<UBmrHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

// Returns the generated map
ABmrGeneratedMap& ABmrGeneratedMap::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	ABmrGeneratedMap* GeneratedMap = UBmrGeneratedMapSubsystem::Get(OptionalWorldContext).GetGeneratedMap();
	checkf(GeneratedMap, TEXT("%s: ERROR: 'GeneratedMap' is null"), *FString(__FUNCTION__));
	return *GeneratedMap;
}

// Attempts to return the generated map, nullptr otherwise
ABmrGeneratedMap* ABmrGeneratedMap::GetGeneratedMap(const UObject* OptionalWorldContext /* = nullptr*/)
{
	constexpr bool bWarnIfNull = false;
	const UBmrGeneratedMapSubsystem* Subsystem = UBmrGeneratedMapSubsystem::GetGeneratedMapSubsystem(OptionalWorldContext);
	return Subsystem ? Subsystem->GetGeneratedMap(bWarnIfNull) : nullptr;
}

// Returns the settings used for generating the map
const FBmrGeneratedMapSettings& ABmrGeneratedMap::GetGenerationSetting() const
{
	if (bOverrideGenerationSettings
	    && OverriddenGenerationSettings.IsValid())
	{
		return OverriddenGenerationSettings;
	}

	if (const UBmrGeneratedMapDataAsset* GeneratedMapDataAsset = UDalSubsystem::GetDataAsset<UBmrGeneratedMapDataAsset>())
	{
		return GeneratedMapDataAsset->GetGenerationSettings();
	}

	return FBmrGeneratedMapSettings::Empty;
}

/*********************************************************************************************
 * Spawn
 ********************************************************************************************* */

// Spawns level actor on the Generated Map by the specified type
void ABmrGeneratedMap::SpawnActorByType(EBmrActorType Type, const FBmrCell& Cell, const TFunction<void(UBmrMapComponent&)>& OnSpawned /* = nullptr*/)
{
	if (!HasAuthority()
	    || UBmrCellUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
	    || Type == EAT::None) // nothing to spawn
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(ABmrGeneratedMap::SpawnActorByType);

	// --- Prepare spawn request
	const FOnSpawnCallback OnCompleted = [WeakThis = TWeakObjectPtr(this), OnSpawned](const FPoolObjectData& CreatedObject)
	{
		ABmrGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Setup spawned actor
		AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
		SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
		SpawnedActor.SetOwner(This);

		UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(&SpawnedActor);
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);

		This->AddToGrid(MapComponent);

		if (OnSpawned != nullptr)
		{
			OnSpawned(*MapComponent);
		}
	};

	// --- Spawn actor
	const UClass* ActorClass = UBmrBlueprintFunctionLibrary::GetActorClassByActorType(Type);
	if (!ensureMsgf(ActorClass, TEXT("ASSERT: [%i] %hs:\n'ActorClass' is null for %s type!"), __LINE__, __FUNCTION__, *UEnum::GetValueAsString(Type)))
	{
		return;
	}

	const FPoolObjectHandle Handle = UPoolManagerSubsystem::Get().TakeFromPool(ActorClass, FTransform(Cell), OnCompleted);

	// --- Add Handle if requested spawning, so it can be canceled if regenerate before spawning finished
	checkf(Handle.IsValid(), TEXT("ERROR: [%i] %s:\n'Handle' is not valid!"), __LINE__, *FString(__FUNCTION__));
	MapComponents.FindOrAdd(Handle);
}

// Spawns multiple level actors at once, mostly used for level generation
void ABmrGeneratedMap::SpawnActorsByTypes(const TMap<FBmrCell, EBmrActorType>& ActorsToSpawn, const TFunction<void(const TArray<UBmrMapComponent*>&)>& OnSpawned /* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	// --- Prepare spawn requests
	TArray<FSpawnRequest> InRequests;
	for (const TTuple<FBmrCell, EBmrActorType>& It : ActorsToSpawn)
	{
		const FBmrCell& Cell = It.Key;
		const EBmrActorType& Type = It.Value;

		if (UBmrCellUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
		    || Type == EAT::None) // nothing to spawn
		{
			continue;
		}

		const UClass* ActorClass = UBmrBlueprintFunctionLibrary::GetActorClassByActorType(Type);
		if (ensureMsgf(ActorClass, TEXT("ASSERT: [%i] %hs:\n'ActorClass' is null for %s type!"), __LINE__, __FUNCTION__, *UEnum::GetValueAsString(Type)))
		{
			FSpawnRequest& NewRequestRef = InRequests.Emplace_GetRef(ActorClass);
			NewRequestRef.Transform = FTransform(Cell);
		}
	}

	// --- Prepare On Spawn All callback
	const FOnSpawnAllCallback OnCompleted = [WeakThis = TWeakObjectPtr(this), OnSpawned](const TArray<FPoolObjectData>& CreatedObjects)
	{
		ABmrGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Setup spawned actors
		TArray<UBmrMapComponent*> MapComponents;
		for (const FPoolObjectData& CreatedObject : CreatedObjects)
		{
			AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
			SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
			SpawnedActor.SetOwner(This);

			UBmrMapComponent* MapComponent = UBmrMapComponent::GetMapComponent(&SpawnedActor);
			checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);
			MapComponents.AddUnique(MapComponent);

			This->AddToGrid(MapComponent);
		}

		if (OnSpawned != nullptr)
		{
			OnSpawned(MapComponents);
		}
	};

	// --- Spawn all actors
	TArray<FPoolObjectHandle> Handles;
	UPoolManagerSubsystem::Get().TakeFromPoolArray(Handles, InRequests, OnCompleted);

	// --- Add handles if requested spawning, so they can be canceled if regenerate before spawning finished
	for (const FPoolObjectHandle& HandleIt : Handles)
	{
		MapComponents.FindOrAdd(HandleIt);
	}
}

// Spawns level actor of given type by the specified pattern
void ABmrGeneratedMap::SpawnActorsByPattern(EBmrActorType ActorsType, const TArray<FIntPoint>& Positions)
{
	if (!HasAuthority()
	    || !ensureMsgf(ActorsType != EAT::None, TEXT("ASSERT: [%i] %hs:\n'ActorsType' is None!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!Positions.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'Positions' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Spawn actors by the specified columns (X) and rows (Y)
	TMap<FBmrCell, EBmrActorType> CellsToSpawn;
	for (const FIntPoint& It : Positions)
	{
		FBmrCell Cell = UBmrCellUtilsLibrary::GetCellByPositionOnLevel(It.X, It.Y);
		CellsToSpawn.Emplace(MoveTemp(Cell), ActorsType);
	}
	SpawnActorsByTypes(CellsToSpawn);
}

// Spawns level actors with the specified mesh data on the Generated Map
void ABmrGeneratedMap::SpawnActorWithMesh(EBmrActorType ActorType, const FBmrCell& Cell, const FBmrMeshData& MeshData)
{
	if (ensureMsgf(MeshData.IsValid(), TEXT("ASSERT: [%i] %hs:\n'MeshData' is not valid!"), __LINE__, __FUNCTION__))
	{
		// Server-only visual override: clients fall back to MapComponent::OnAdded's FindFirstRow, player mesh goes through ABmrPlayerState::SetChosenMeshData
		const auto& OnSpawned = [MeshData](UBmrMapComponent& MapComponent)
		{
			MapComponent.TryApplyMeshFromRow(MeshData.RowName);
		};
		SpawnActorByType(ActorType, Cell, OnSpawned);
	}
}

// Adding and attaching the specified Map Component to the Level
void ABmrGeneratedMap::AddToGrid(UBmrMapComponent* AddedComponent)
{
	AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || IS_TRANSIENT(ComponentOwner))
	{
		return;
	}

	// Snap to the nearest cell
	const bool bSnapped = SetNearestCell(AddedComponent);

	if (!bSnapped
	    && MapComponents.Contains(AddedComponent))
	{
		// Is already on the same cell and registered
		return;
	}

	// First, add it to the Pool Manager if level actor was spawned manually
	UPoolManagerSubsystem& PoolManager = UPoolManagerSubsystem::Get();
	FPoolObjectHandle Handle = PoolManager.FindPoolHandleByObject(ComponentOwner);
	if (!Handle.IsValid())
	{
		FPoolObjectData ObjectData(ComponentOwner);
		ObjectData.bIsActive = true;
		ObjectData.Handle = FPoolObjectHandle::NewHandle(ComponentOwner->GetClass());
		const bool bRegistered = PoolManager.RegisterObjectInPool(ObjectData);
		ensureMsgf(bRegistered, TEXT("ASSERT: [%i] %hs:\n'Failed to registered '%s' in the Pool Manager!"), __LINE__, __FUNCTION__, *GetNameSafe(ComponentOwner));
		Handle = ObjectData.Handle;
	}

	const FBmrCell& Cell = AddedComponent->GetCell();
	if (!ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is zero")))
	{
		// Actor is already added on the level
		return;
	}

	AddToGridDragged(AddedComponent);

	// If found, means was spawned before, otherwise is taken from pool
	FBmrMapComponentSpec& NewSpec = MapComponents.FindOrAdd(Handle);
	NewSpec.MapComponent = AddedComponent;
	NewSpec.Cell = Cell;
	MapComponents.MarkItemDirty(NewSpec);

	// Find transform
	FRotator ActorRotation = GetActorRotation();
	const EBmrActorType ActorType = AddedComponent->GetActorType();
	if (TO_FLAG(ActorType) & TO_FLAG(EAT::Box | EAT::Wall))
	{
		// Random rotate if is Box or Wall
		static constexpr float RotationMultiplier = 90.f;
		static constexpr int32 MinRange = 1;
		static constexpr int32 MaxRange = 4;
		ActorRotation.Yaw += FMath::RandRange(MinRange, MaxRange) * RotationMultiplier;
	}
	const FVector ActorLocation{Cell.X(), Cell.Y(), Cell.Z() + UBmrGeneratedMapDataAsset::Get().GetActorsHeightOffset()};

	if (ActorType != EAT::Player)
	{
		// Attach to the Generated Map actor (except player since they have to move on level freely)
		if (!ComponentOwner->IsAttachedTo(this))
		{
			ComponentOwner->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}

		// Locate actor on cell (except pawns, which are handled by Mover)
		ComponentOwner->SetActorTransform(FTransform(ActorRotation, ActorLocation, FVector::OneVector));
	}

	// Notify listeners
	AddedComponent->OnAdded();
}

// Client-only method to resolve a newly spawned Map Component
void ABmrGeneratedMap::ResolveSpawnedMapComponent(UBmrMapComponent& AddedComponent)
{
	if (HasAuthority())
	{
		// Is not a client
		return;
	}

	const FBmrCell Cell = UBmrCellUtilsLibrary::SnapActorOnLevel(AddedComponent.GetOwner());
	FBmrMapComponentSpec* FoundSpec = MapComponents.Find(Cell);
	if (!FoundSpec)
	{
		// Not found, nothing to resolve
		return;
	}

	const bool bNeedsResolve = !FoundSpec->MapComponent;
	if (bNeedsResolve)
	{
		// The reference was replicated before the component was spawned
		// Validate the reference to fixup broken reference
		// It intentionally affects only this client, but not server and other players
		FoundSpec->MapComponent = &AddedComponent;
		FoundSpec->PostReplicatedAdd(MapComponents);
	}
}

// Internal method called on both server and client to increment the replication token whenever any level actor is spawned
void ABmrGeneratedMap::IncrementReplicationToken()
{
	++MapComponents.LocalReplicationToken;

	if (MapComponents.LocalReplicationToken == GenerateLevelActorsToken)
	{
		// All level actors completed spawn (on server) or all replicated (on client)
		OnGeneratedLevelActors.Broadcast();
	}
}

// Internal client-only callback when GenerateLevelActorsToken is replicated from server
void ABmrGeneratedMap::OnRep_GenerateLevelActorsToken()
{
	if (GenerateLevelActorsToken == 0)
	{
		// Server started a new generation, reset local token to count from 0
		MapComponents.LocalReplicationToken = 0;
		return;
	}

	// Catch-up: all actors arrived before the token packet, so the match in IncrementReplicationToken was missed
	if (MapComponents.LocalReplicationToken >= GenerateLevelActorsToken)
	{
		OnGeneratedLevelActors.Broadcast();
	}
}

/*********************************************************************************************
 * Destroy
 ********************************************************************************************* */

// Destroy all actors from the set of cells
void ABmrGeneratedMap::DestroyLevelActorsOnCells(const FBmrCells& Cells, UObject* DestroyCauser /* = nullptr*/)
{
	if (!HasAuthority()
	    || !MapComponents.Num()
	    || !Cells.Num())
	{
		return;
	}

	// Iterate and destroy
	for (int32 Index = MapComponents.Num() - 1; Index >= 0; --Index)
	{
		if (!MapComponents.IsValidIndex(Index)) // the element already was removed
		{
			continue;
		}

		UBmrMapComponent* MapComponentIt = MapComponents[Index];
		const AActor* OwnerIt = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		const bool bCellIsOnGrid = MapComponentIt && Cells.Contains(MapComponentIt->GetCell());
		if (!OwnerIt // if is null, destroy that object from the array
		    || bCellIsOnGrid)
		{
			// Remove from the array
			// First removing, because after the box destroying the powerup can be spawned and starts searching for an empty cell
			// MapComponentIt can be invalid here
			DestroyLevelActor(MapComponentIt, DestroyCauser);
		}
	}
	MapComponents.Items.Shrink();
}

// Destroy level actor by specified Map Component from the level
void ABmrGeneratedMap::DestroyLevelActor(UBmrMapComponent* MapComponent, UObject* DestroyCauser /* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(ABmrGeneratedMap::DestroyLevelActor);

	AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!ComponentOwner)
	{
		return;
	}

	// First, unregister from the level
	RemoveFromGrid(MapComponent, DestroyCauser);

	MapComponents.Remove(MapComponent);

	// Perform destroy itself
	UPoolManagerSubsystem* PoolManager = UPoolManagerSubsystem::GetPoolManager(this);
	if (PoolManager
	    && PoolManager->ContainsObjectInPool(ComponentOwner))
	{
		PoolManager->ReturnToPool(ComponentOwner);
	}
	else
	{
		// Pool Manager can be null on level destroy
		ComponentOwner->Destroy();
	}

	DestroyLevelActorDragged(MapComponent);

	// Notify listeners after destroying being performed
	MapComponent->OnPostRemoved(DestroyCauser);
}

// Destroys level actor by specified handle
void ABmrGeneratedMap::DestroyLevelActorByHandle(const FPoolObjectHandle& Handle, UObject* DestroyCauser)
{
	if (!HasAuthority()
	    || !ensureMsgf(Handle.IsValid(), TEXT("ASSERT: [%i] %s:\n'Handle' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const FBmrMapComponentSpec* MapComponentData = MapComponents.Find(Handle);
	if (!ensureMsgf(MapComponentData, TEXT("ASSERT: [%i] %s:\n'MapComponentData' is not found by given handle!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (UBmrMapComponent* MapComponent = MapComponentData->MapComponent)
	{
		DestroyLevelActor(MapComponent, DestroyCauser);
		return;
	}

	// Map component was not found, it could be not spawned, but in spawn request in queue
	UPoolManagerSubsystem::Get().ReturnToPool(Handle);

	MapComponents.Remove(Handle);
}

// Destroy all level actors of given type from the level
void ABmrGeneratedMap::DestroyLevelActorsByType(EBmrActorType ActorsType, UObject* DestroyCauser)
{
	if (!HasAuthority()
	    || ActorsType == EAT::None)
	{
		return;
	}

	const FBmrCells ExistingActorCells = UBmrCellUtilsLibrary::GetAllCellsWithActors(TO_FLAG(ActorsType));
	DestroyLevelActorsOnCells(ExistingActorCells);
}

// Is called before Destroy happening, which only unregisters the Map Component
void ABmrGeneratedMap::RemoveFromGrid(UBmrMapComponent* MapComponent, UObject* RemoveCauser)
{
	if (!HasAuthority()
	    || !MapComponent)
	{
		return;
	}

	MapComponent->OnPreRemoved(RemoveCauser);

	// Invalidate spec now (removal will be performed if only DestroyLevelActor called)
	if (FBmrMapComponentSpec* Spec = MapComponents.Find(MapComponent))
	{
		Spec->Cell = FBmrCell::InvalidCell;
		MapComponents.MarkItemDirty(*Spec);
	}
}

// Applies the snapped cell to the specified Map Component
bool ABmrGeneratedMap::SetNearestCell(UBmrMapComponent* InMapComponent)
{
	const AActor* LevelActor = InMapComponent ? InMapComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || !LevelActor)
	{
		return false;
	}

	// In game, snap the actor to the current cell (even if the cell is occupied by others)
	const FBmrCell& LastCell = InMapComponent->GetCell();
	FBmrCell FoundFreeCell = UBmrCellUtilsLibrary::SnapActorOnLevel(LevelActor);
	const bool bIsSnappedGame = FoundFreeCell != LastCell;

	/// In editor world, always perform additional snaps
	const bool bIsSnappedDragged = SetNearestCellDragged(InMapComponent, /*InOut*/ FoundFreeCell);

	if (!bIsSnappedGame && !bIsSnappedDragged)
	{
		// The actor is already aligned on the level
		return false;
	}

	InMapComponent->SetCell(FoundFreeCell);

	// If Map Component is added to the level (spec exists), then update its cell for the replication purpose
	// It might be not added to the level yet by design, so it will be added and updated later
	if (FBmrMapComponentSpec* Spec = MapComponents.Find(InMapComponent))
	{
		Spec->Cell = FoundFreeCell;
		MapComponents.MarkItemDirty(*Spec);
	}

	return true;
}

// Takes transform and returns aligned copy allowed to be used as actor transform for this map
FTransform ABmrGeneratedMap::ActorTransformToGridTransform(const FTransform& ActorTransform)
{
	FTransform NewTransform = FTransform::Identity;

	// Align location snapping to the grid size
	FVector NewLocation = FVector::ZeroVector;
	if (!Get().GetGenerationSetting().LockOnZero)
	{
		NewLocation = FBmrCell::SnapCell(ActorTransform.GetLocation());
	}
	NewTransform.SetLocation(NewLocation);

	// Align rotation allowing only yaw axis
	const FRotator NewRotation(0.f, ActorTransform.GetRotation().Rotator().Yaw, 0.f);
	NewTransform.SetRotation(NewRotation.Quaternion());

	// Align scale to have only unpaired integers for XY and always 1 for Z
	FIntPoint NewLevelSize(ActorTransform.GetScale3D().X, ActorTransform.GetScale3D().Y);
	if (NewLevelSize.X % 2 != 1) // Width (columns) must be unpaired
	{
		NewLevelSize.X += 1;
	}
	if (NewLevelSize.Y % 2 != 1) // Length (rows) must be unpaired-
	{
		NewLevelSize.Y += 1;
	}
	constexpr int32 MapScaleZ = 1;
	NewTransform.SetScale3D(FVector(NewLevelSize, MapScaleZ));

	return MoveTemp(NewTransform);
}

// Returns ability system component that is used to manage environmental abilities, crash if nullptr
UAbilitySystemComponent& ABmrGeneratedMap::GetAbilitySystemComponentChecked() const
{
	checkf(AbilitySystemComponent, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);
	return *AbilitySystemComponent;
}

// Broadcasts WorldASC_Ready event once per world session when the ASC is available
void ABmrGeneratedMap::TryBroadcastWorldASCReady()
{
	if (UGlobalMessageSubsystem::HasBroadcastedMessage(BmrGameplayTags::Event::WorldASC_Ready))
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = BmrGameplayTags::Event::WorldASC_Ready;
	Payload.Instigator = this;
	Payload.OptionalObject = AbilitySystemComponent;
	UGlobalMessageSubsystem::BroadcastGlobalMessage(Payload);
}

/*********************************************************************************************
 * Overrides and events
 ********************************************************************************************* */

// Safe initialization called both in editor and in game when the Generated Map subsystem is fully ready
void ABmrGeneratedMap::OnGeneratedMapReady_Implementation()
{
	OnConstructionGeneratedMap(GetActorTransform());
}

// Called when an instance of this class is placed (in editor) or spawned
void ABmrGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		// In editor, construct this actor for preview
		OnConstructionGeneratedMap(Transform);
	}
}

// Initialize this Generated Map actor, could be called multiple times
void ABmrGeneratedMap::OnConstructionGeneratedMap_Implementation(const FTransform& Transform)
{
	if (IS_TRANSIENT(this)
	    || !ensureMsgf(!Transform.GetScale3D().IsZero(), TEXT("ASSERT: [%i] %hs:\n'Transform' has zero scale!"), __LINE__, __FUNCTION__))
	{
		return;
	}

#if WITH_EDITOR // [GEditor]
	UBmrGeneratedMapSubsystem::Get().SetGeneratedMap(this);
	TryBroadcastWorldASCReady();
	if (GEditor // Can be bound before editor is loaded
	    && !UBmrUnrealEdEngine::GOnAnyDataAssetChanged.IsBoundToObject(this))
	{
		// Should be bind in construction in a case of object reconstructing after blueprint compile
		UBmrUnrealEdEngine::GOnAnyDataAssetChanged.AddUObject(this, &ThisClass::RerunConstructionScripts);
	}
#endif // WITH_EDITOR [GEditor]

	if (!UBmrGeneratedMapSubsystem::Get().IsGeneratedMapReady())
	{
		// Attempted to construct before Generated Map is fully initialized, will reconstruct later
		return;
	}

	// Create the background blueprint child actor
	if (CollisionComponent // Is accessible
	    && !CollisionComponent->GetChildActor()) // Is not created yet
	{
		const TSubclassOf<AActor> CollisionsAssetClass = UBmrGeneratedMapDataAsset::Get().GetCollisionsAssetClass();
		CollisionComponent->SetChildActorClass(CollisionsAssetClass);
		CollisionComponent->CreateChildActor();
	}

	// If generation settings are overridden, validate the generator
	if (bOverrideGenerationSettings
	    && !OverriddenGenerationSettings.Generator)
	{
		OverriddenGenerationSettings.Generator = UBmrGeneratedMapDataAsset::Get().GetGenerationSettings().Generator;
	}

	// Align transform and build cells
	BuildGridCells(Transform);

	// Actors generation
	GenerateLevelActors();

	// Update camera position
	if (CameraComponent)
	{
		CameraComponent->UpdateLocation();
	}
}

// Called right before components are initialized, only called during gameplay
void ABmrGeneratedMap::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register Generated Map to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// This is called only in the gameplay before calling begin play to generate level actors
void ABmrGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (IS_TRANSIENT(this)) // the Generated Map is transient
	{
		return;
	}

	// Update the gameplay GeneratedMap reference in the singleton library
	UBmrGeneratedMapSubsystem::Get().SetGeneratedMap(this);
	TryBroadcastWorldASCReady();

	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, this);

	OnConstructionGeneratedMap(GetActorTransform());

	UGlobalMessageSubsystem::CallOrStartListeningForGlobalMessage(BmrGameplayTags::Event::GameState_Changed, this, &ThisClass::OnGameStateChanged);

	// During the game, OnConstruction is not called when location, rotation or scale is changed, so bind to listen transform updates
	checkf(RootComponent, TEXT("ERROR: [%i] %hs:\n'RootComponent' is null!"), __LINE__, __FUNCTION__);
	RootComponent->TransformUpdated.AddLambda([WeakThis = TWeakObjectPtr(this)](USceneComponent*, EUpdateTransformFlags, ETeleportType)
	{
		if (ABmrGeneratedMap* This = WeakThis.Get())
		{
			This->OnConstructionGeneratedMap(This->GetActorTransform());
		}
	});
}

// Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending
void ABmrGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this)
	    && HasAuthority())
	{
		// Destroy level actors
		UPoolManagerSubsystem::Get().EmptyAllPools();

		// Destroy level actors in internal arrays
		const int32 MapComponentsNum = MapComponents.Num();
		for (int32 Index = MapComponentsNum - 1; Index >= 0; --Index)
		{
			DestroyLevelActor(MapComponents[Index]);
		}

#if WITH_EDITOR // [IsEditorNotPieWorld]
		if (FEditorUtilsLibrary::IsEditorNotPieWorld())
		{
			// Remove editor bound delegates
			UBmrUnrealEdEngine::GOnAnyDataAssetChanged.RemoveAll(this);
		}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
	}

	if (RootComponent)
	{
		RootComponent->TransformUpdated.RemoveAll(this);
	}

	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::Destroyed();
}

// Returns properties that are replicated for the lifetime of the actor channel
void ABmrGeneratedMap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MapComponents, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GenerateLevelActorsToken, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AbilitySystemComponent, Params);
}

// Listen game states to generate level actors
void ABmrGeneratedMap::OnGameStateChanged_Implementation(const FGameplayEventData& Payload)
{
	if (!HasAuthority())
	{
		return;
	}

	// Regenerate level actors when:
	// 1. Returning to the menu (ensures the world resets properly)
	// 2. Restarting the game (ensures a fresh state between matches)
	// Note: GenerateLevelActors() internally guards against re-entry via bIsCurrentlyGenerating
	if (Payload.InstigatorTags.HasAny(FBmrGameStateTag::Menu | FBmrGameStateTag::GameStarting))
	{
		GenerateLevelActors();
	}
}

/*********************************************************************************************
 * Generation
 ********************************************************************************************* */

// Allows to override the default settings used for generating the m
void ABmrGeneratedMap::SetOverriddenGenerationSettings(bool bEnableOverride, const FBmrGeneratedMapSettings& InSettings)
{
	if (!HasAuthority())
	{
		return;
	}

	bOverrideGenerationSettings = bEnableOverride;

	if (!bEnableOverride)
	{
		// Disable override and cleanup
		OverriddenGenerationSettings = FBmrGeneratedMapSettings::Empty;
		return;
	}

	OverriddenGenerationSettings = InSettings;

	if (!OverriddenGenerationSettings.Generator)
	{
		// Generator is optional and might be not set, use the default one from Data Asset then
		OverriddenGenerationSettings.Generator = UBmrGeneratedMapDataAsset::Get().GetGenerationSettings().Generator;
	}
}

// Allows to change the size for generated map in runtime, it will automatically regenerate the level
void ABmrGeneratedMap::SetLevelSize(const FIntPoint& LevelSize)
{
	if (!HasAuthority()
	    || !ensureMsgf(LevelSize.GetMin() > 0, TEXT("%hs: 'LevelSize' is invalid: %s"), __FUNCTION__, *LevelSize.ToString()))
	{
		return;
	}

	SetActorScale3D(FVector(LevelSize.X, LevelSize.Y, 1.f));

	GenerateLevelActors();
}

// Spawns and fills the Grid Array values by level actors
void ABmrGeneratedMap::GenerateLevelActors()
{
	if (!ensureMsgf(!LocalGridCells.IsEmpty(), TEXT("ASSERT: [%i] %hs:\nThere are no cells on the Generated Map!"), __LINE__, __FUNCTION__)
	    || !HasAuthority()
	    || bIsCurrentlyGenerating)
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(ABmrGeneratedMap::GenerateLevelActors);

	bIsCurrentlyGenerating = true;

	DestroyAllLevelActors();

	FBmrGeneratorData GeneratorData;
	GeneratorData.AllCells = LocalGridCells;
	GeneratorData.MapScale = FIntPoint(GetActorScale3D().X, GetActorScale3D().Y);
	GeneratorData.AllCellPositions = FBmrCell::GetPositionsByCellsOnGrid(LocalGridCells, GeneratorData.MapScale.X);
	GeneratorData.GenerationSettings = GetGenerationSetting();
	GeneratorData.DraggedCells = DraggedCells;

	// Compute cells on background thread and finish with spawning on the game thread (copy data for thread safety)
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis = TWeakObjectPtr(this), InData = MoveTemp(GeneratorData)]() mutable -> void
	{
		TMap<FBmrCell, EBmrActorType> ActorsToSpawn = GenerateLevelActors_StartAsync(MoveTemp(InData));
		AsyncTaskGameThread(WeakThis.Get(), [WeakThis, InActorsToSpawn = MoveTemp(ActorsToSpawn)]() mutable -> void
		{
			if (ABmrGeneratedMap* This = WeakThis.Get())
			{
				This->GenerateLevelActors_Finish(MoveTemp(InActorsToSpawn));
			}
		});
	});
}

// Internal method to compute cells on background thread
TMap<FBmrCell, EBmrActorType> ABmrGeneratedMap::GenerateLevelActors_StartAsync(FBmrGeneratorData&& GeneratorData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ABmrGeneratedMap::GenerateLevelActors_StartAsync);

	UBmrCellsGenerator_Base* Generator = GeneratorData.GenerationSettings.Generator;
	ensureMsgf(Generator, TEXT("ASSERT: [%i] %hs:\n'Generator' is not set in the Data Asset!"), __LINE__, __FUNCTION__);
	return Generator ? Generator->GenerateLevel(MoveTemp(GeneratorData)) : TMap<FBmrCell, EBmrActorType>{};
}

// Internal method to finish with spawning on the game thread
void ABmrGeneratedMap::GenerateLevelActors_Finish(TMap<FBmrCell, EBmrActorType>&& ActorsToSpawn)
{
	const UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	// --- Part 2: Spawning ---

	const TFunction<void(const TArray<UBmrMapComponent*>&)> OnSpawned = [WeakThis = TWeakObjectPtr(this)](const TArray<UBmrMapComponent*>& MapComponents)
	{
		ABmrGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Replicate the token to clients, so they can track when all actors completed generation
		This->GenerateLevelActorsToken = This->MapComponents.LocalReplicationToken;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, GenerateLevelActorsToken, This);

		This->bIsCurrentlyGenerating = false;
		This->OnGeneratedLevelActors.Broadcast();
	};

	SpawnActorsByTypes(ActorsToSpawn, OnSpawned);
}

// Destroys all currently spawned level actors and resets generation tokens, leaving the grid empty
void ABmrGeneratedMap::DestroyAllLevelActors()
{
	if (!HasAuthority())
	{
		return;
	}

	// Iterate it by handles to cancel spawning even if the actor is not spawned yet
	TArray<FBmrMapComponentSpec>& MapComponentsToDestroy = MapComponents.Items;
	for (int32 Idx = MapComponentsToDestroy.Num() - 1; Idx >= 0; Idx--)
	{
		DestroyLevelActorByHandle(MapComponentsToDestroy[Idx].PoolObjectHandle);
	}
	checkf(MapComponentsToDestroy.IsEmpty(), TEXT("ERROR: [%i] %hs:\n'MapComponentsToDestroy' is not empty after removing all!"), __LINE__, __FUNCTION__);

	// Reset both tokens so the next generation counts from 0 on server and signals clients to reset too
	MapComponents.LocalReplicationToken = 0;
	GenerateLevelActorsToken = 0;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, GenerateLevelActorsToken, this);

	AdditionalDangerousCells.Reset();
}

// Align transform and build cells
void ABmrGeneratedMap::BuildGridCells(const FTransform& Transform)
{
	const FTransform NewGridTransform = ActorTransformToGridTransform(Transform);
	if (UBmrCellUtilsLibrary::GetLevelGridTransform().Equals(NewGridTransform))
	{
		// Do not rebuild if the transform is the same
		return;
	}

	const FBmrCells NewGridCells = FBmrCell::MakeCellGridByTransform(NewGridTransform);

	ScaleDraggedCellsOnGrid(FBmrCells{LocalGridCells}, NewGridCells);

	if (HasAuthority())
	{
		SetActorTransform(NewGridTransform);
	}

	LocalGridCells = NewGridCells.Array();
}

/*********************************************************************************************
 * Dragged Level Actors
 ********************************************************************************************* */

// Returns true if specified map component has non-generated owner that is manually dragged to the scene
bool ABmrGeneratedMap::IsDraggedMapComponent(const UBmrMapComponent* MapComponent) const
{
	if (!MapComponent)
	{
		return false;
	}

	const EBmrActorType ActorType = MapComponent->GetActorType();
	const FBmrCell& Cell = MapComponent->GetCell();
	if (MapComponent->GetActorType() == EAT::None
	    || Cell.IsInvalidCell())
	{
		return false;
	}

	const EBmrActorType* FoundCell = DraggedCells.Find(Cell);
	return FoundCell && *FoundCell == ActorType;
}

// Scales dragged cells according new grid if sizes are different
void ABmrGeneratedMap::ScaleDraggedCellsOnGrid(const FBmrCells& OriginalGrid, const FBmrCells& NewGrid)
{
	if (OriginalGrid.IsEmpty()
	    || OriginalGrid.Num() == NewGrid.Num())
	{
		// Do not scale if the sizes are the same
		return;
	}

	const FBmrCells CornerCells = FBmrCell::GetCornerCellsOnGrid(NewGrid);
	const FBmrCells NewGridWithoutCorners = NewGrid.Difference(CornerCells);

	for (TTuple<FBmrCell, EBmrActorType>& DraggedCellRefIt : DraggedCells)
	{
		FBmrCell& CurrentCellRef = DraggedCellRefIt.Key;
		const FBmrCell ScaledCell = FBmrCell::ScaleCellToNewGrid(CurrentCellRef, CornerCells);
		CurrentCellRef = FBmrCell::GetCellArrayNearest(NewGridWithoutCorners, ScaledCell);
	}
}

// The dragged version of the Add To Grid function to add the dragged actor on the level
void ABmrGeneratedMap::AddToGridDragged(UBmrMapComponent* AddedComponent)
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	const AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!ComponentOwner
	    || ComponentOwner->bIsEditorPreviewActor)
	{
		return;
	}

	const bool bIsDraggedMapComponent = IsDraggedMapComponent(AddedComponent);
	const bool bIsSelectedInEditor = ComponentOwner->IsSelectedInEditor();
	if (!bIsSelectedInEditor && !bIsDraggedMapComponent)
	{
		// Is not dragged
		return;
	}

	// Is dragged actor
	// The game that is not started yet and owner locates in the editor.
	// Each similar not dragged actor that is generated by Generated Map is preview actor, so is not dragged.

	const FBmrCell& DraggedCell = AddedComponent->GetCell();
	if (!DraggedCells.Contains(DraggedCell))
	{
		DraggedCells.Emplace(DraggedCell, AddedComponent->GetActorType());
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor
bool ABmrGeneratedMap::SetNearestCellDragged(const UBmrMapComponent* MapComponent, FBmrCell& InOutCell)
{
#if !WITH_EDITOR
	return false;
#else // WITH_EDITOR [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld()
	    || !MapComponent
	    || InOutCell.IsInvalidCell())
	{
		return false;
	}

	const FBmrCell& CurrentCell = MapComponent->GetCell();
	if (CurrentCell != InOutCell)
	{
		// Is dragged to new cell, find free cell without any actors, so dragged actor will never overlap
		InOutCell = UBmrCellUtilsLibrary::GetNearestFreeCell(InOutCell);
	}

	if (IsDraggedMapComponent(MapComponent))
	{
		DraggedCells.Remove(CurrentCell);
		DraggedCells.Emplace(InOutCell, MapComponent->GetActorType());
	}

	return true;
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Destroy Level Actor function to hide the dragged actor from the level
void ABmrGeneratedMap::DestroyLevelActorDragged(const UBmrMapComponent* MapComponent)
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld()
	    || !MapComponent
	    || !IS_TRANSIENT(MapComponent->GetOwner())) // Never destroy valid actors, hide them instead
	{
		return;
	}

	if (IsDraggedMapComponent(MapComponent))
	{
		const FBmrCell& Cell = MapComponent->GetCell();
		DraggedCells.Remove(Cell);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}