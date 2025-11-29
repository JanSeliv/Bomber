// Copyright (c) Yevhenii Selivanov.

#include "GeneratedMap.h"

// Bomber
#include "AbilitySystem/Attributes/BmrHealthAttributeSet.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/GeneratedMapDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "Generators/BmrCellsGenerator_Base.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "PoolManagerSubsystem.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"

#if WITH_EDITOR
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#include "MyUnrealEdEngine.h"
#endif

// UE
#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GeneratedMap)

// Empty generation settings instance
const FGeneratedMapSettings FGeneratedMapSettings::Empty = FGeneratedMapSettings();

/* ---------------------------------------------------
 *		Generated Map public functions
 * --------------------------------------------------- */

// Sets default values
AGeneratedMap::AGeneratedMap()
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
	static constexpr float NewNewUpdateFrequency = 10.f;
	SetNetUpdateFrequency(NewNewUpdateFrequency);
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
	CollisionComponentInternal = CreateDefaultSubobject<UChildActorComponent>(TEXT("Collision Component"));
	CollisionComponentInternal->SetupAttachment(RootComponent);

	// Default camera class
	CameraComponentInternal = CreateDefaultSubobject<UMyCameraComponent>(TEXT("Camera Component"));
	CameraComponentInternal->SetupAttachment(RootComponent);

	// Create ASC on Generated Map for environmental abilities and level-related mods (e.g., environmental explosions)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	HealthSetInternal = CreateDefaultSubobject<UBmrHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

// Returns the generated map
AGeneratedMap& AGeneratedMap::Get(const UObject* OptionalWorldContext /* = nullptr*/)
{
	AGeneratedMap* GeneratedMap = UGeneratedMapSubsystem::Get(OptionalWorldContext).GetGeneratedMap();
	checkf(GeneratedMap, TEXT("%s: ERROR: 'GeneratedMap' is null"), *FString(__FUNCTION__));
	return *GeneratedMap;
}

// Attempts to return the generated map, nullptr otherwise
AGeneratedMap* AGeneratedMap::GetGeneratedMap(const UObject* OptionalWorldContext /* = nullptr*/)
{
	constexpr bool bWarnIfNull = false;
	const UGeneratedMapSubsystem* Subsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem(OptionalWorldContext);
	return Subsystem ? Subsystem->GetGeneratedMap(bWarnIfNull) : nullptr;
}

// Returns the settings used for generating the map
const FGeneratedMapSettings& AGeneratedMap::GetGenerationSetting() const
{
	if (bOverrideGenerationSettingsInternal
	    && OverriddenGenerationSettingsInternal.IsValid())
	{
		return OverriddenGenerationSettingsInternal;
	}

	if (const UGeneratedMapDataAsset* GeneratedMapDataAsset = UDataAssetsContainer::GetGeneratedMapDataAsset())
	{
		return GeneratedMapDataAsset->GetGenerationSettings();
	}

	return FGeneratedMapSettings::Empty;
}

// Allows to override the default settings used for generating the m
void AGeneratedMap::SetOverriddenGenerationSettings(bool bEnableOverride, const FGeneratedMapSettings& InSettings)
{
	if (!HasAuthority())
	{
		return;
	}

	bOverrideGenerationSettingsInternal = bEnableOverride;

	if (!bEnableOverride)
	{
		// Disable override and cleanup
		OverriddenGenerationSettingsInternal = FGeneratedMapSettings::Empty;
		return;
	}

	OverriddenGenerationSettingsInternal = InSettings;

	if (!OverriddenGenerationSettingsInternal.Generator)
	{
		// Generator is optional and might be not set, use the default one from Data Asset then
		OverriddenGenerationSettingsInternal.Generator = UGeneratedMapDataAsset::Get().GetGenerationSettings().Generator;
	}
}

// Allows to change the size for generated map in runtime, it will automatically regenerate the level
void AGeneratedMap::SetLevelSize(const FIntPoint& LevelSize)
{
	if (!HasAuthority()
	    || !ensureMsgf(LevelSize.GetMin() > 0, TEXT("%hs: 'LevelSize' is invalid: %s"), __FUNCTION__, *LevelSize.ToString()))
	{
		return;
	}

	SetActorScale3D(FVector(LevelSize.X, LevelSize.Y, 1.f));
}

/*********************************************************************************************
 * Spawn
 ********************************************************************************************* */

// Spawns level actor on the Generated Map by the specified type
void AGeneratedMap::SpawnActorByType(EActorType Type, const FCell& Cell, const TFunction<void(UMapComponent&)>& OnSpawned /* = nullptr*/)
{
	if (!HasAuthority()
	    || UCellsUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
	    || Type == EAT::None) // nothing to spawn
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::SpawnActorByType);

	// --- Prepare spawn request
	const FOnSpawnCallback OnCompleted = [WeakThis = TWeakObjectPtr(this), OnSpawned](const FPoolObjectData& CreatedObject)
	{
		AGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Setup spawned actor
		AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
		SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
		SpawnedActor.SetOwner(This);

		UMapComponent* MapComponent = UMapComponent::GetMapComponent(&SpawnedActor);
		checkf(MapComponent, TEXT("ERROR: [%i] %hs:\n'MapComponent' is null!"), __LINE__, __FUNCTION__);

		This->AddToGrid(MapComponent);

		if (OnSpawned != nullptr)
		{
			OnSpawned(*MapComponent);
		}
	};

	// --- Spawn actor
	const UClass* ClassToSpawn = UDataAssetsContainer::GetActorClassByType(Type);
	const FPoolObjectHandle Handle = UPoolManagerSubsystem::Get().TakeFromPool(ClassToSpawn, FTransform(Cell), OnCompleted);

	// --- Add Handle if requested spawning, so it can be canceled if regenerate before spawning finished
	checkf(Handle.IsValid(), TEXT("ERROR: [%i] %s:\n'Handle' is not valid!"), __LINE__, *FString(__FUNCTION__));
	MapComponentsInternal.FindOrAdd(Handle);
}

// Spawns multiple level actors at once, mostly used for level generation
void AGeneratedMap::SpawnActorsByTypes(const TMap<FCell, EActorType>& ActorsToSpawn, const TFunction<void(const TArray<UMapComponent*>&)>& OnSpawned /* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	// --- Prepare spawn requests
	TArray<FSpawnRequest> InRequests;
	for (const TTuple<FCell, EActorType>& It : ActorsToSpawn)
	{
		const FCell& Cell = It.Key;
		const EActorType& Type = It.Value;

		if (UCellsUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
		    || Type == EAT::None) // nothing to spawn
		{
			continue;
		}

		FSpawnRequest& NewRequestRef = InRequests.Emplace_GetRef(UDataAssetsContainer::GetActorClassByType(Type));
		NewRequestRef.Transform = FTransform(Cell);
	}

	// --- Prepare On Spawn All callback
	const FOnSpawnAllCallback OnCompleted = [WeakThis = TWeakObjectPtr(this), OnSpawned](const TArray<FPoolObjectData>& CreatedObjects)
	{
		AGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Setup spawned actors
		TArray<UMapComponent*> MapComponents;
		for (const FPoolObjectData& CreatedObject : CreatedObjects)
		{
			AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
			SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
			SpawnedActor.SetOwner(This);

			UMapComponent* MapComponent = UMapComponent::GetMapComponent(&SpawnedActor);
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
		MapComponentsInternal.FindOrAdd(HandleIt);
	}
}

// Spawns level actor of given type by the specified pattern
void AGeneratedMap::SpawnActorsByPattern(EActorType ActorsType, const TArray<FIntPoint>& Positions)
{
	if (!HasAuthority()
	    || !ensureMsgf(ActorsType != EAT::None, TEXT("ASSERT: [%i] %hs:\n'ActorsType' is None!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(!Positions.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'Positions' is empty!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// Spawn actors by the specified columns (X) and rows (Y)
	TMap<FCell, EActorType> CellsToSpawn;
	for (const FIntPoint& It : Positions)
	{
		FCell Cell = UCellsUtilsLibrary::GetCellByPositionOnLevel(It.X, It.Y);
		CellsToSpawn.Emplace(MoveTemp(Cell), ActorsType);
	}
	SpawnActorsByTypes(CellsToSpawn);
}

// Spawns level actors with the specified mesh data on the Generated Map
void AGeneratedMap::SpawnActorWithMesh(EActorType ActorType, const FCell& Cell, const FBmrMeshData& MeshData)
{
	if (ensureMsgf(MeshData.IsValid(), TEXT("ASSERT: [%i] %hs:\n'MeshData' is not valid!"), __LINE__, __FUNCTION__))
	{
		const auto& OnSpawned = [MeshData](UMapComponent& MapComponent)
		{
			MapComponent.SetReplicatedMeshData(MeshData);
		};
		SpawnActorByType(ActorType, Cell, OnSpawned);
	}
}

// Adding and attaching the specified Map Component to the Level
void AGeneratedMap::AddToGrid(UMapComponent* AddedComponent)
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
	    && MapComponentsInternal.Contains(AddedComponent))
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

	const FCell& Cell = AddedComponent->GetCell();
	if (!ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is zero")))
	{
		// Actor is already added on the level
		return;
	}

	AddToGridDragged(AddedComponent);

	// If found, means was spawned before, otherwise is taken from pool
	FMapComponentSpec& NewSpec = MapComponentsInternal.FindOrAdd(Handle);
	NewSpec.MapComponent = AddedComponent;
	NewSpec.Cell = Cell;
	MapComponentsInternal.MarkItemDirty(NewSpec);

	// Find transform
	FRotator ActorRotation = GetActorRotation();
	const EActorType ActorType = AddedComponent->GetActorType();
	if (TO_FLAG(ActorType) & TO_FLAG(EAT::Box | EAT::Wall))
	{
		// Random rotate if is Box or Wall
		static constexpr float RotationMultiplier = 90.f;
		static constexpr int32 MinRange = 1;
		static constexpr int32 MaxRange = 4;
		ActorRotation.Yaw += FMath::RandRange(MinRange, MaxRange) * RotationMultiplier;
	}
	static constexpr float HeightAdditive = 100.f;
	const FVector ActorLocation{Cell.X(), Cell.Y(), Cell.Z() + HeightAdditive};

	// Attach to the Generated Map actor
	if (!ComponentOwner->IsAttachedTo(this)
	    && ActorType != EAT::Player)
	{
		// Do not attach players to level since they have to move on level freely
		ComponentOwner->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	// Locate actor on cell
	ComponentOwner->SetActorTransform(FTransform(ActorRotation, ActorLocation, FVector::OneVector));

	// Notify listeners
	AddedComponent->OnAdded();
}

// Client-only method to resolve a newly spawned Map Component
void AGeneratedMap::ResolveSpawnedMapComponent(UMapComponent& AddedComponent)
{
	if (HasAuthority())
	{
		// Is not a client
		return;
	}

	const FCell Cell = UCellsUtilsLibrary::SnapActorOnLevel(AddedComponent.GetOwner());
	FMapComponentSpec* FoundSpec = MapComponentsInternal.Find(Cell);
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
		FoundSpec->PostReplicatedAdd(MapComponentsInternal);
	}
}

// Internal method called on both server and client to increment the replication token whenever any level actor is spawned
void AGeneratedMap::IncrementReplicationToken()
{
	++MapComponentsInternal.LocalReplicationToken;

	if (MapComponentsInternal.LocalReplicationToken == GenerateLevelActorsTokenInternal)
	{
		// All level actors completed spawn (on server) or all replicated (on client)
		OnGeneratedLevelActors.Broadcast();
	}
}

/*********************************************************************************************
 * Destroy
 ********************************************************************************************* */

// Destroy all actors from the set of cells
void AGeneratedMap::DestroyLevelActorsOnCells(const FCells& Cells, UObject* DestroyCauser /* = nullptr*/)
{
	if (!HasAuthority()
	    || !MapComponentsInternal.Num()
	    || !Cells.Num())
	{
		return;
	}

	// Iterate and destroy
	for (int32 Index = MapComponentsInternal.Num() - 1; Index >= 0; --Index)
	{
		if (!MapComponentsInternal.IsValidIndex(Index)) // the element already was removed
		{
			continue;
		}

		UMapComponent* MapComponentIt = MapComponentsInternal[Index];
		const AActor* OwnerIt = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		const bool bCellIsOnGrid = MapComponentIt && Cells.Contains(MapComponentIt->GetCell());
		if (!OwnerIt // if is null, destroy that object from the array
		    || bCellIsOnGrid)
		{
			// Remove from the array
			// First removing, because after the box destroying the item can be spawned and starts searching for an empty cell
			// MapComponentIt can be invalid here
			DestroyLevelActor(MapComponentIt, DestroyCauser);
		}
	}
	MapComponentsInternal.Items.Shrink();
}

// Destroy level actor by specified Map Component from the level
void AGeneratedMap::DestroyLevelActor(UMapComponent* MapComponent, UObject* DestroyCauser /* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::DestroyLevelActor);

	AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!ComponentOwner)
	{
		return;
	}

	// First, unregister from the level
	RemoveFromGrid(MapComponent, DestroyCauser);

	MapComponentsInternal.Remove(MapComponent);

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
void AGeneratedMap::DestroyLevelActorByHandle(const FPoolObjectHandle& Handle, UObject* DestroyCauser)
{
	if (!HasAuthority()
	    || !ensureMsgf(Handle.IsValid(), TEXT("ASSERT: [%i] %s:\n'Handle' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const FMapComponentSpec* MapComponentData = MapComponentsInternal.Find(Handle);
	if (!ensureMsgf(MapComponentData, TEXT("ASSERT: [%i] %s:\n'MapComponentData' is not found by given handle!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	if (UMapComponent* MapComponent = MapComponentData->MapComponent)
	{
		DestroyLevelActor(MapComponent, DestroyCauser);
		return;
	}

	// Map component was not found, it could be not spawned, but in spawn request in queue
	UPoolManagerSubsystem::Get().ReturnToPool(Handle);

	MapComponentsInternal.Remove(Handle);
}

// Destroy all level actors of given type from the level
void AGeneratedMap::DestroyLevelActorsByType(EActorType ActorsType, UObject* DestroyCauser)
{
	if (!HasAuthority()
	    || ActorsType == EAT::None)
	{
		return;
	}

	const FCells ExistingActorCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(ActorsType));
	DestroyLevelActorsOnCells(ExistingActorCells);
}

// Is called before Destroy happening, which only unregisters the Map Component
void AGeneratedMap::RemoveFromGrid(UMapComponent* MapComponent, UObject* RemoveCauser)
{
	if (!HasAuthority()
	    || !MapComponent)
	{
		return;
	}

	MapComponent->OnPreRemoved(RemoveCauser);

	// Invalidate spec now (removal will be performed if only DestroyLevelActor called)
	if (FMapComponentSpec* Spec = MapComponentsInternal.Find(MapComponent))
	{
		Spec->Cell = FCell::InvalidCell;
		MapComponentsInternal.MarkItemDirty(*Spec);
	}
}

// Applies the snapped cell to the specified Map Component
bool AGeneratedMap::SetNearestCell(UMapComponent* MapComponent)
{
	const AActor* LevelActor = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || !LevelActor)
	{
		return false;
	}

	// In game, snap the actor to the current cell (even if the cell is occupied by others)
	const FCell& LastCell = MapComponent->GetCell();
	FCell FoundFreeCell = UCellsUtilsLibrary::SnapActorOnLevel(LevelActor);
	const bool bIsSnappedGame = FoundFreeCell != LastCell;

	/// In editor world, always perform additional snaps
	const bool bIsSnappedDragged = SetNearestCellDragged(MapComponent, /*InOut*/ FoundFreeCell);

	if (!bIsSnappedGame && !bIsSnappedDragged)
	{
		// The actor is already aligned on the level
		return false;
	}

	MapComponent->SetCell(FoundFreeCell);

	// If Map Component is added to the level (spec exists), then update its cell for the replication purpose
	// It might be not added to the level yet by design, so it will be added and updated later
	if (FMapComponentSpec* Spec = MapComponentsInternal.Find(MapComponent))
	{
		Spec->Cell = FoundFreeCell;
		MapComponentsInternal.MarkItemDirty(*Spec);
	}

	return true;
}

// Takes transform and returns aligned copy allowed to be used as actor transform for this map
FTransform AGeneratedMap::ActorTransformToGridTransform(const FTransform& ActorTransform)
{
	FTransform NewTransform = FTransform::Identity;

	// Align location snapping to the grid size
	FVector NewLocation = FVector::ZeroVector;
	if (!Get().GetGenerationSetting().LockOnZero)
	{
		NewLocation = FCell::SnapCell(ActorTransform.GetLocation());
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
UAbilitySystemComponent& AGeneratedMap::GetAbilitySystemComponentChecked() const
{
	checkf(AbilitySystemComponent, TEXT("ERROR: [%i] %hs:\n'AbilitySystemComponent' is null!"), __LINE__, __FUNCTION__);
	return *AbilitySystemComponent;
}

/* ---------------------------------------------------
 *		Generated Map protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned
void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		// In editor, construct this actor for preview
		OnConstructionGeneratedMap(Transform);
	}
}

// Initialize this Generated Map actor, could be called multiple times
void AGeneratedMap::OnConstructionGeneratedMap_Implementation(const FTransform& Transform)
{
	if (IS_TRANSIENT(this)
	    || !ensureMsgf(!Transform.GetScale3D().IsZero(), TEXT("ASSERT: [%i] %hs:\n'Transform' has zero scale!"), __LINE__, __FUNCTION__))
	{
		return;
	}

#if WITH_EDITOR // [GEditor]
	UGeneratedMapSubsystem::Get().SetGeneratedMap(this);
	if (GEditor // Can be bound before editor is loaded
	    && !UMyUnrealEdEngine::GOnAnyDataAssetChanged.IsBoundToObject(this))
	{
		// Should be bind in construction in a case of object reconstructing after blueprint compile
		UMyUnrealEdEngine::GOnAnyDataAssetChanged.AddUObject(this, &ThisClass::RerunConstructionScripts);
	}
#endif // WITH_EDITOR [GEditor]

	// Create the background blueprint child actor
	if (CollisionComponentInternal // Is accessible
	    && !CollisionComponentInternal->GetChildActor()) // Is not created yet
	{
		const TSubclassOf<AActor> CollisionsAssetClass = UGeneratedMapDataAsset::Get().GetCollisionsAssetClass();
		CollisionComponentInternal->SetChildActorClass(CollisionsAssetClass);
		CollisionComponentInternal->CreateChildActor();
	}

	// If generation settings are overridden, validate the generator
	if (bOverrideGenerationSettingsInternal
	    && !OverriddenGenerationSettingsInternal.Generator)
	{
		OverriddenGenerationSettingsInternal.Generator = UGeneratedMapDataAsset::Get().GetGenerationSettings().Generator;
	}

	// Align transform and build cells
	BuildGridCells(Transform);

	// Actors generation
	GenerateLevelActors();

	// Update camera position
	if (CameraComponentInternal)
	{
		CameraComponentInternal->UpdateLocation();
	}
}

// Called right before components are initialized, only called during gameplay
void AGeneratedMap::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register Generated Map to let to be implemented by game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// This is called only in the gameplay before calling begin play to generate level actors
void AGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (IS_TRANSIENT(this)) // the Generated Map is transient
	{
		return;
	}

	// Update the gameplay GeneratedMap reference in the singleton library
	UGeneratedMapSubsystem::Get().SetGeneratedMap(this);

	GetAbilitySystemComponentChecked().InitAbilityActorInfo(this, this);

	OnConstructionGeneratedMap(GetActorTransform());

	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// During the game, OnConstruction is not called when location, rotation or scale is changed, so bind to listen transform updates
	checkf(RootComponent, TEXT("ERROR: [%i] %hs:\n'RootComponent' is null!"), __LINE__, __FUNCTION__);
	RootComponent->TransformUpdated.AddLambda([WeakThis = TWeakObjectPtr(this)](USceneComponent*, EUpdateTransformFlags, ETeleportType)
	{
		if (AGeneratedMap* This = WeakThis.Get())
		{
			This->OnConstructionGeneratedMap(This->GetActorTransform());
		}
	});
}

// Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending
void AGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this)
	    && HasAuthority())
	{
		// Destroy level actors
		UPoolManagerSubsystem::Get().EmptyAllPools();

		// Destroy level actors in internal arrays
		const int32 MapComponentsNum = MapComponentsInternal.Num();
		for (int32 Index = MapComponentsNum - 1; Index >= 0; --Index)
		{
			DestroyLevelActor(MapComponentsInternal[Index]);
		}

#if WITH_EDITOR // [IsEditorNotPieWorld]
		if (FEditorUtilsLibrary::IsEditorNotPieWorld())
		{
			// Remove editor bound delegates
			UMyUnrealEdEngine::GOnAnyDataAssetChanged.RemoveAll(this);
		}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
	}

	if (RootComponent)
	{
		RootComponent->TransformUpdated.RemoveAll(this);
	}

	Super::Destroyed();
}

// Returns properties that are replicated for the lifetime of the actor channel
void AGeneratedMap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MapComponentsInternal, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GenerateLevelActorsTokenInternal, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AbilitySystemComponent, Params);
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	if (!ensureMsgf(!LocalGridCellsInternal.IsEmpty(), TEXT("ASSERT: [%i] %hs:\nThere are no cells on the Generated Map!"), __LINE__, __FUNCTION__)
	    || !HasAuthority()
	    || bIsCurrentlyGeneratingInternal)
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::GenerateLevelActors);

	bIsCurrentlyGeneratingInternal = true;

	// Destroy all actors first
	// Iterate it by handles to cancel spawning even if the actor is not spawned yet
	TArray<FMapComponentSpec>& MapComponentsToDestroy = MapComponentsInternal.Items;
	for (int32 Idx = MapComponentsToDestroy.Num() - 1; Idx >= 0; Idx--)
	{
		DestroyLevelActorByHandle(MapComponentsToDestroy[Idx].PoolObjectHandle);
	}
	checkf(MapComponentsToDestroy.IsEmpty(), TEXT("ERROR: [%i] %hs:\n'MapComponentsToDestroy' is not empty after removing all!"), __LINE__, __FUNCTION__);

	AdditionalDangerousCells.Reset();

	FBmrGeneratorData GeneratorData;
	GeneratorData.AllCells = LocalGridCellsInternal;
	GeneratorData.MapScale = FIntPoint(GetActorScale3D().X, GetActorScale3D().Y);
	GeneratorData.AllCellPositions = FCell::GetPositionsByCellsOnGrid(LocalGridCellsInternal, GeneratorData.MapScale.X);
	GeneratorData.GenerationSettings = GetGenerationSetting();
	GeneratorData.DraggedCells = DraggedCellsInternal;

	// Compute cells on background thread and finish with spawning on the game thread (copy data for thread safety)
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis = TWeakObjectPtr(this), InData = MoveTemp(GeneratorData)]() mutable -> void
	{
		TMap<FCell, EActorType> ActorsToSpawn = GenerateLevelActors_StartAsync(MoveTemp(InData));
		AsyncTaskGameThread(WeakThis.Get(), [WeakThis, InActorsToSpawn = MoveTemp(ActorsToSpawn)]() mutable -> void
		{
			if (AGeneratedMap* This = WeakThis.Get())
			{
				This->GenerateLevelActors_Finish(MoveTemp(InActorsToSpawn));
			}
		});
	});
}

// Internal method to compute cells on background thread
TMap<FCell, EActorType> AGeneratedMap::GenerateLevelActors_StartAsync(FBmrGeneratorData&& GeneratorData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::GenerateLevelActors_StartAsync);

	UBmrCellsGenerator_Base* Generator = GeneratorData.GenerationSettings.Generator;
	ensureMsgf(Generator, TEXT("ASSERT: [%i] %hs:\n'Generator' is not set in the Data Asset!"), __LINE__, __FUNCTION__);
	return Generator ? Generator->GenerateLevel(MoveTemp(GeneratorData)) : TMap<FCell, EActorType>{};
}

// Internal method to finish with spawning on the game thread
void AGeneratedMap::GenerateLevelActors_Finish(TMap<FCell, EActorType>&& ActorsToSpawn)
{
	// --- Part 2: Spawning ---

	const TFunction<void(const TArray<UMapComponent*>&)> OnSpawned = [WeakThis = TWeakObjectPtr(this)](const TArray<UMapComponent*>& MapComponents)
	{
		AGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Replicate the token to clients, so they can track when all actors completed generation
		This->GenerateLevelActorsTokenInternal = This->MapComponentsInternal.LocalReplicationToken;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, GenerateLevelActorsTokenInternal, This);

		This->bIsCurrentlyGeneratingInternal = false;
		This->OnGeneratedLevelActors.Broadcast();
	};

	SpawnActorsByTypes(ActorsToSpawn, OnSpawned);
}

// Listen game states to generate level actors
void AGeneratedMap::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (!HasAuthority())
	{
		return;
	}

	const ECurrentGameState PreviousGameState = AMyGameStateBase::GetPreviousGameState();
	switch (CurrentGameState)
	{
		case ECGS::Menu: // Fallthrough
		case ECGS::GameStarting:
		{
			if (PreviousGameState == ECGS::None)
			{
				// Game is starting for the first time, do nothing, the level is expected to be generated in the OnConstruction
				break;
			}

			// Regenerate level actors when:
			// 1. Returning to the menu (ensures the world resets properly)
			// 2. Restarting the game (ensures a fresh state between matches)
			const bool bNowInMenu = CurrentGameState == ECGS::Menu;
			const bool bRestartedMatch = PreviousGameState != ECGS::Menu;
			if (bNowInMenu
			    || bRestartedMatch)
			{
				GenerateLevelActors();
			}
			break;
		}

		default:
			break;
	}
}

// Align transform and build cells
void AGeneratedMap::BuildGridCells(const FTransform& Transform)
{
	const FTransform NewGridTransform = ActorTransformToGridTransform(Transform);
	if (UCellsUtilsLibrary::GetLevelGridTransform().Equals(NewGridTransform))
	{
		// Do not rebuild if the transform is the same
		return;
	}

	const FCells NewGridCells = FCell::MakeCellGridByTransform(NewGridTransform);

	ScaleDraggedCellsOnGrid(FCells{LocalGridCellsInternal}, NewGridCells);

	if (HasAuthority())
	{
		SetActorTransform(NewGridTransform);
	}

	LocalGridCellsInternal = NewGridCells.Array();
}

/*********************************************************************************************
 * Dragged Level Actors
 ********************************************************************************************* */

// Returns true if specified map component has non-generated owner that is manually dragged to the scene
bool AGeneratedMap::IsDraggedMapComponent(const UMapComponent* MapComponent) const
{
	if (!MapComponent)
	{
		return false;
	}

	const EActorType ActorType = MapComponent->GetActorType();
	const FCell& Cell = MapComponent->GetCell();
	if (MapComponent->GetActorType() == EAT::None
	    || Cell.IsInvalidCell())
	{
		return false;
	}

	const EActorType* FoundCell = DraggedCellsInternal.Find(Cell);
	return FoundCell && *FoundCell == ActorType;
}

// Scales dragged cells according new grid if sizes are different
void AGeneratedMap::ScaleDraggedCellsOnGrid(const FCells& OriginalGrid, const FCells& NewGrid)
{
	if (OriginalGrid.IsEmpty()
	    || OriginalGrid.Num() == NewGrid.Num())
	{
		// Do not scale if the sizes are the same
		return;
	}

	const FCells CornerCells = FCell::GetCornerCellsOnGrid(NewGrid);
	const FCells NewGridWithoutCorners = NewGrid.Difference(CornerCells);

	for (TTuple<FCell, EActorType>& DraggedCellRefIt : DraggedCellsInternal)
	{
		FCell& CurrentCellRef = DraggedCellRefIt.Key;
		const FCell ScaledCell = FCell::ScaleCellToNewGrid(CurrentCellRef, CornerCells);
		CurrentCellRef = FCell::GetCellArrayNearest(NewGridWithoutCorners, ScaledCell);
	}
}

// The dragged version of the Add To Grid function to add the dragged actor on the level
void AGeneratedMap::AddToGridDragged(UMapComponent* AddedComponent)
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

	const FCell& DraggedCell = AddedComponent->GetCell();
	if (!DraggedCellsInternal.Contains(DraggedCell))
	{
		DraggedCellsInternal.Emplace(DraggedCell, AddedComponent->GetActorType());
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor
bool AGeneratedMap::SetNearestCellDragged(const UMapComponent* MapComponent, FCell& InOutCell)
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

	const FCell& CurrentCell = MapComponent->GetCell();
	if (CurrentCell != InOutCell)
	{
		// Is dragged to new cell, find free cell without any actors, so dragged actor will never overlap
		InOutCell = UCellsUtilsLibrary::GetNearestFreeCell(InOutCell);
	}

	if (IsDraggedMapComponent(MapComponent))
	{
		DraggedCellsInternal.Remove(CurrentCell);
		DraggedCellsInternal.Emplace(InOutCell, MapComponent->GetActorType());
	}

	return true;
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Destroy Level Actor function to hide the dragged actor from the level
void AGeneratedMap::DestroyLevelActorDragged(const UMapComponent* MapComponent)
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
		const FCell& Cell = MapComponent->GetCell();
		DraggedCellsInternal.Remove(Cell);
	}
#endif // WITH_EDITOR [IsEditorNotPieWorld]
}