// Copyright (c) Yevhenii Selivanov.

#include "GeneratedMap.h"
//---
#include "PoolManagerSubsystem.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "DataAssets/DataAssetsContainer.h"
#include "DataAssets/GeneratedMapDataAsset.h"
#include "GameFramework/MyGameStateBase.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Subsystems/GeneratedMapSubsystem.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/LevelActorsUtilsLibrary.h"
//---
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "MyUnrealEdEngine.h"
#include "MyEditorUtilsLibraries/EditorUtilsLibrary.h"
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(GeneratedMap)

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

#if WITH_EDITOR	 //[Editor]
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;
#endif	//WITH_EDITOR [Editor]

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
}

// Returns the generated map
AGeneratedMap& AGeneratedMap::Get(const UObject* OptionalWorldContext/* = nullptr*/)
{
	AGeneratedMap* GeneratedMap = UGeneratedMapSubsystem::Get(OptionalWorldContext).GetGeneratedMap();
	checkf(GeneratedMap, TEXT("%s: ERROR: 'GeneratedMap' is null"), *FString(__FUNCTION__));
	return *GeneratedMap;
}

// Attempts to return the generated map, nullptr otherwise
AGeneratedMap* AGeneratedMap::GetGeneratedMap(const UObject* OptionalWorldContext/* = nullptr*/)
{
	constexpr bool bWarnIfNull = false;
	const UGeneratedMapSubsystem* Subsystem = UGeneratedMapSubsystem::GetGeneratedMapSubsystem(OptionalWorldContext);
	return Subsystem ? Subsystem->GetGeneratedMap(bWarnIfNull) : nullptr;
}

// Returns the settings used for generating the map
const FGeneratedMapSettings& AGeneratedMap::GetGenerationSetting() const
{
	if (bOverrideGenerationSettingsInternal)
	{
		return OverriddenGenerationSettingsInternal;
	}

	if (const UGeneratedMapDataAsset* GeneratedMapDataAsset = UDataAssetsContainer::GetGeneratedMapDataAsset())
	{
		return GeneratedMapDataAsset->GetGenerationSettings();
	}

	static constexpr FGeneratedMapSettings DefaultSettings{};
	return DefaultSettings;
}

// Allows to change the size for generated map in runtime, it will automatically regenerate the level
void AGeneratedMap::SetLevelSize(const FIntPoint& LevelSize)
{
	if (!HasAuthority()
	    || !ensureMsgf(LevelSize.GetMin() > 0, TEXT("%s: 'LevelSize' is invalid: %s"), *FString(__FUNCTION__), *LevelSize.ToString()))
	{
		return;
	}

	SetActorScale3D(FVector(LevelSize.X, LevelSize.Y, 1.f));
}

// Getting an array of cells by four sides of an input center cell and type of breaks
void AGeneratedMap::GetSidesCells(
	FCells& OutCells,
	const FCell& Cell,
	EPathType Pathfinder,
	int32 SideLength,
	int32 DirectionsBitmask,
	bool bBreakInputCells) const
{
	const int32 MaxWidth = UCellsUtilsLibrary::GetCellColumnsNumOnLevel();
	if (!ensureMsgf(MaxWidth, TEXT("ASSERT: Level has zero width (Scale.X)"))
	    || !ensureMsgf(DirectionsBitmask, TEXT("ASSERT: 'DirectionsBitmask' is not set"))
	    || !ensureMsgf(SideLength > 0, TEXT("ASSERT: 'SideLength' is less than 1"))
	    || !ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is invalid")))
	{
		return;
	}

	const bool bIsAnyPath = Pathfinder == EPathType::Any;

	// ----- Walls definition -----
	FCells Walls;
	bool bBreakOnWalls = !bIsAnyPath && !OutCells.Num();
	if (bBreakOnWalls)
	{
		Walls = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Wall));
	}
	else if (bBreakInputCells) // specified OutCells is not empty, these cells break lines as the Wall behavior
	{
		bBreakOnWalls = true;
		Walls = OutCells; // these cells break lines as the Wall behavior, don't empty specified array
	}
	else // !bBreakOnWalls && !bBreakInputCells
	{
		OutCells.Empty(); // should empty array in order to return only sides cells
	}

	// the index of the specified cell
	const int32 C0 = LocalGridCellsInternal.IndexOfByPredicate([&Cell](const FCell& InCell) { return InCell == Cell; });
	if (C0 == INDEX_NONE) // if index was found and cell is contained in the array
	{
		return;
	}

	// ----- A path without obstacles -----
	FCells Obstacles;
	const bool bBreakOnObstacles = !bIsAnyPath && Pathfinder != EPathType::Explosion;
	if (bBreakOnObstacles) // if is the request to find the path without Bombs/Boxes
	{
		Obstacles = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Bomb | EAT::Box));
	}

	// ----- Secure: a path without players -----
	FCells PlayersCells;
	const bool bBreakOnPlayers = Pathfinder == EPathType::Secure;
	if (bBreakOnPlayers) // if is the request to find the path without players cells.
	{
		PlayersCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Player));
	}

	// ----- A path without explosions -----
	FCells DangerousCells = AdditionalDangerousCells;
	const bool bBreakOnExplosions = Pathfinder == EPathType::Safe || Pathfinder == EPathType::Secure;
	if (bBreakOnExplosions) // if is the request to find the path without explosions.
	{
		DangerousCells.Append(UCellsUtilsLibrary::GetAllExplosionCells());
	}

	// ----- The specified cell adding -----
	if (bBreakOnExplosions == false        // can be danger
	    || !DangerousCells.Contains(Cell)) // is not dangerous cell
	{
		OutCells.Emplace(Cell);
	}

	// ----- Cells finding -----
	for (int8 bIsY = 0; bIsY <= 1; ++bIsY) // 0(X-raw direction) and 1(Y-column direction)
	{
		const int32 PositionC0 = bIsY ? /*Y-column*/ C0 % MaxWidth : C0 / MaxWidth /*raw*/;
		for (int8 SideMultiplier = -1; SideMultiplier <= 1; SideMultiplier += 2) // -1(Left|Down) and 1(Right|Up)
		{
			const FVector& VectorDirection = bIsY ? FVector::BackwardVector : FVector::RightVector;
			const FCell CellDirection = VectorDirection * FVector(SideMultiplier);
			const ECellDirection EnumDirection = FCell::GetCellDirection(CellDirection);
			if (!EnumHasAnyFlags(EnumDirection, TO_ENUM(ECellDirection, DirectionsBitmask)))
			{
				continue;
			}

			for (int8 i = 1; i <= SideLength; ++i)
			{
				int32 Distance = i * SideMultiplier;
				if (bIsY)
				{
					Distance *= MaxWidth;
				}
				const int32 FoundIndex = C0 + Distance;
				if (PositionC0 != (bIsY ? FoundIndex % MaxWidth : FoundIndex / MaxWidth) // PositionC0 != PositionX
				    || !LocalGridCellsInternal.IsValidIndex(FoundIndex))                 // is not in range
				{
					break; // to the next side
				}

				const FCell FoundCell = LocalGridCellsInternal[FoundIndex];

				if (bBreakOnWalls
				    && Walls.Contains(FoundCell))
				{
					// cell contains a wall
					break;
				}

				if (bBreakOnObstacles
				    && Obstacles.Contains(FoundCell))
				{
					// cell contains an obstacle (Bombs/Boxes)
					break;
				}

				if (bBreakOnPlayers
				    && PlayersCells.Contains(FoundCell))
				{
					// cell contains a player
					break;
				}

				if (bBreakOnExplosions
				    && DangerousCells.Contains(FoundCell))
				{
					// cell contains an explosion
					break;
				}

				OutCells.Emplace(FoundCell);
			} // Cells iterating
		}     // Each side iterating: -1(Left|Down) and 1(Right|Up)
	}         // Each direction iterating: 0(X-raw) and 1(Y-column)
}

// Returns true if any player is able to reach all specified cells by any any path
bool AGeneratedMap::DoesPathExistToCells(const FCells& CellsToFind, const FCells& OptionalPathBreakers/* = FCell::EmptyCells*/) const
{
	FCells InOutSideCells = OptionalPathBreakers;
	if (OptionalPathBreakers.IsEmpty())
	{
		// Include walls to prevent finding way through their cells
		InOutSideCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Wall));
	}

	// Contains all cells need to find their side cells
	checkf(!LocalGridCellsInternal.IsEmpty(), TEXT("ERROR: [%i] %hs:\n'LocalGridCellsInternal' is empty!"), __LINE__, __FUNCTION__);
	FCells CellsToIterate{LocalGridCellsInternal[0]};

	FCells FoundCells = FCell::EmptyCells;
	while (CellsToIterate.Num())
	{
		// Cache all previous side cells
		const FCells PrevSideCells = InOutSideCells;

		for (const FCell& CellIt : CellsToIterate)
		{
			constexpr int32 MaxInteger = TNumericLimits<int32>::Max();
			constexpr bool bBreakInputCells = true;
			// InOutAllFoundCells include as wall cells as well all previous iterated cells
			GetSidesCells(/*InOut*/InOutSideCells, CellIt, EPathType::Explosion, MaxInteger, TO_FLAG(ECellDirection::All), bBreakInputCells);
		}

		// Extract newly found cells
		CellsToIterate = InOutSideCells.Difference(PrevSideCells);

		const FCells NotFoundCells = CellsToFind.Difference(FoundCells);
		FoundCells = CellsToIterate.Intersect(NotFoundCells).Union(FoundCells);
		if (FoundCells.Includes(CellsToFind))
		{
			return true;
		}
	}

	return false;
}

/*********************************************************************************************
 * Spawn
 ********************************************************************************************* */

// Spawns level actor on the Generated Map by the specified type
void AGeneratedMap::SpawnActorByType(EActorType Type, const FCell& Cell, const TFunction<void(AActor*)>& OnSpawned/* = nullptr*/)
{
	if (!HasAuthority()
	    || UCellsUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
	    || Type == EAT::None)                                                         // nothing to spawn
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::SpawnActorByType);

	// --- Prepare spawn request
	const TWeakObjectPtr<ThisClass> WeakThis(this);
	const FOnSpawnCallback OnCompleted = [WeakThis, OnSpawned](const FPoolObjectData& CreatedObject)
	{
		AGeneratedMap* GeneratedMap = WeakThis.Get();
		if (!GeneratedMap)
		{
			return;
		}

		// Setup spawned actor
		AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
		SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
		SpawnedActor.SetOwner(GeneratedMap);

		if (OnSpawned != nullptr)
		{
			OnSpawned(&SpawnedActor);
		}
	};

	// --- Spawn actor
	const UClass* ClassToSpawn = UDataAssetsContainer::GetActorClassByType(Type);
	const FPoolObjectHandle Handle = UPoolManagerSubsystem::Get().TakeFromPool(ClassToSpawn, FTransform(Cell), OnCompleted);

	// --- Add Handle if requested spawning, so it can be canceled if regenerate before spawning finished
	checkf(Handle.IsValid(), TEXT("ERROR: [%i] %s:\n'Handle' is not valid!"), __LINE__, *FString(__FUNCTION__));
	MapComponentsInternal.FindOrAdd(Handle);
}

// Spawns many level actors at once, used for level generation
void AGeneratedMap::SpawnActorsByTypes(const TMap<FCell, EActorType>& ActorsToSpawn)
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
		    || Type == EAT::None)                                                      // nothing to spawn
		{
			continue;
		}

		FSpawnRequest& NewRequestRef = InRequests.Emplace_GetRef(UDataAssetsContainer::GetActorClassByType(Type));
		NewRequestRef.Transform = FTransform(Cell);
	}

	// --- Prepare On Spawn All callback
	TWeakObjectPtr<ThisClass> WeakThis(this);
	const FOnSpawnAllCallback OnCompleted = [WeakThis](const TArray<FPoolObjectData>& CreatedObjects)
	{
		AGeneratedMap* This = WeakThis.Get();
		if (!This)
		{
			return;
		}

		// Setup spawned actors
		for (const FPoolObjectData& CreatedObject : CreatedObjects)
		{
			AActor& SpawnedActor = CreatedObject.GetChecked<AActor>();
			SpawnedActor.SetFlags(RF_Transient); // Do not save generated actors into the map
			SpawnedActor.SetOwner(This);
		}

		This->MapComponentsInternal.MarkArrayDirty();

		This->OnGeneratedLevelActors.Broadcast();
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

// Adding and attaching the specified Map Component to the Level
void AGeneratedMap::AddToGrid(UMapComponent* AddedComponent)
{
	AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || !ComponentOwner
	    || !ComponentOwner->HasAuthority())
	{
		return;
	}

	const FPoolObjectHandle& Handle = UPoolManagerSubsystem::Get().FindPoolHandleByObject(ComponentOwner);
	if (!ensureMsgf(Handle.IsValid(), TEXT("ASSERT: [%i] %s:\n'Handle' is not valid, this object has to be known by Pool Manager!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	const FCell& Cell = AddedComponent->GetCell();
	if (!ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is zero")))
	{
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
}

// The intersection of (OutCells ∩ ActorsTypesBitmask).
void AGeneratedMap::IntersectCellsByTypes(
	FCells& InOutCells,
	int32 ActorsTypesBitmask,
	bool bIntersectAllIfEmpty) const
{
	if (LocalGridCellsInternal.IsEmpty())
	{
		// nothing to intersect
		return;
	}

	if (!bIntersectAllIfEmpty && !InOutCells.Num())
	{
		// should not intersect with all existed cells but the specified array is empty
		return;
	}

	if (!ActorsTypesBitmask)
	{
		// Find all empty grid cell locations where non of actors are present
		const FCells AllEmptyCells = FCells(
			LocalGridCellsInternal.FilterByPredicate([&MapComponents = MapComponentsInternal](const FCell& CellIt)
			{
				return !MapComponents.Contains(CellIt);
			}));

		if (InOutCells.Num())
		{
			InOutCells = InOutCells.Intersect(AllEmptyCells);
		}
		else
		{
			InOutCells = AllEmptyCells;
		}

		return;
	}

	FMapComponents BitmaskedComponents;
	ULevelActorsUtilsLibrary::GetLevelActors(BitmaskedComponents, ActorsTypesBitmask);
	if (!BitmaskedComponents.Num())
	{
		InOutCells.Empty(); // nothing found, returns empty OutCells array
		return;
	}

	FCells BitmaskedCells;
	for (const UMapComponent* MapCompIt : BitmaskedComponents)
	{
		if (MapCompIt)
		{
			BitmaskedCells.Emplace(MapCompIt->GetCell());
		}
	}

	if (InOutCells.Num())
	{
		InOutCells = InOutCells.Intersect(BitmaskedCells);
	}
	else
	{
		InOutCells = BitmaskedCells;
	}
}

/*********************************************************************************************
 * Destroy
 ********************************************************************************************* */

// Destroy all actors from the set of cells
void AGeneratedMap::DestroyLevelActorsOnCells(const FCells& Cells, UObject* DestroyCauser/* = nullptr*/)
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
	MapComponentsInternal.MarkArrayDirty();

	if (OnPostDestroyedLevelActors.IsBound())
	{
		// Broadcast about already destroyed actors
		OnPostDestroyedLevelActors.Broadcast(Cells);
	}
}

// Destroy level actor by specified Map Component from the level
void AGeneratedMap::DestroyLevelActor(UMapComponent* MapComponent, UObject* DestroyCauser/* = nullptr*/)
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

	if (AMyGameStateBase::GetCurrentGameState() == ECGS::InGame
	    && !ComponentOwner->CanBeDamaged())
	{
		// Do not destroy in-game actor during the play session if required
		return;
	}

	MapComponentsInternal.Remove(MapComponent);

	// Notify listeners right before destroying and reset the actor
	MapComponent->OnPreRemoved(DestroyCauser);

	// Deactivate the iterated owner
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

// Applies the snapped cell to the specified Map Component
bool AGeneratedMap::SetNearestCell(UMapComponent* MapComponent)
{
	AActor* LevelActor = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || !LevelActor)
	{
		return false;
	}

	// Snap the actor to the current cell (even if the cell is occupied by others)
	const FCell& LastCell = MapComponent->GetCell();
	FCell FoundFreeCell = UCellsUtilsLibrary::SnapActorOnLevel(LevelActor);
	if (LastCell.IsValid()
	    && FoundFreeCell == LastCell)
	{
		// The actor is already aligned on the level
		return false;
	}

#if WITH_EDITOR //[IsEditorNotPieWorld]
	if (UUtilsLibrary::IsEditorNotPieWorld())
	{
		// In editor world, always move to the free cell without any actors, so dragged actor will never overlap
		FoundFreeCell = UCellsUtilsLibrary::GetNearestFreeCell(LevelActor->GetActorLocation());

		SetNearestCellDragged(MapComponent, FoundFreeCell);
	}
#endif //WITH_EDITOR [IsEditorNotPieWorld]

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
#endif //WITH_EDITOR [GEditor]

	// Create the background blueprint child actor
	if (CollisionComponentInternal                       // Is accessible
	    && !CollisionComponentInternal->GetChildActor()) // Is not created yet
	{
		const TSubclassOf<AActor> CollisionsAssetClass = UGeneratedMapDataAsset::Get().GetCollisionsAssetClass();
		CollisionComponentInternal->SetChildActorClass(CollisionsAssetClass);
		CollisionComponentInternal->CreateChildActor();
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
#endif //WITH_EDITOR [IsEditorNotPieWorld]
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
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	if (!ensureMsgf(!LocalGridCellsInternal.IsEmpty(), TEXT("ASSERT: [%i] %hs:\nThere are no cells on the Generated Map!"), __LINE__, __FUNCTION__)
	    || !HasAuthority())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(AGeneratedMap::GenerateLevelActors);

	// Destroy all actors first
	// Iterate it by handles to cancel spawning even if the actor is not spawned yet
	TArray<FMapComponentSpec>& MapComponentsToDestroy = MapComponentsInternal.Items;
	for (int32 Idx = MapComponentsToDestroy.Num() - 1; Idx >= 0; Idx--)
	{
		DestroyLevelActorByHandle(MapComponentsToDestroy[Idx].PoolObjectHandle);
	}
	checkf(MapComponentsToDestroy.IsEmpty(), TEXT("ERROR: [%i] %s:\n'MapComponentsToDestroy' is not empty after removing all!"), __LINE__, *FString(__FUNCTION__));

	AdditionalDangerousCells.Reset();

	// Calls before generation preview actors to updating of all dragged to the Generated Map actors
	FCells DraggedCells = FCell::EmptyCells;
	FCells DraggedWalls = FCell::EmptyCells;
	FCells DraggedItems = FCell::EmptyCells;
	for (const TTuple<FCell, EActorType>& It : DraggedCellsInternal)
	{
		const FCell& Cell = It.Key;
		const EActorType ActorType = It.Value;

		SpawnActorByType(ActorType, Cell);

		// Store to avoid generation on their cells
		DraggedCells.Emplace(Cell);
		if (ActorType == EActorType::Wall)
		{
			DraggedWalls.Emplace(Cell);
		}
		else if (ActorType == EActorType::Item)
		{
			DraggedItems.Emplace(Cell);
		}
	}

	/* Steps:
	*
	* Part 0: Actors random filling to the ArrayToGenerate.
	* 0.1) Finding all symmetrical cells for each iterated cell;
	*
	* Part 1: Checking if there is a path to the each bone. If not, go to the 0 step.
	*
	* Part 2: Spawning these actors
	*/

	const FGeneratedMapSettings& GenerationSettings = GetGenerationSetting();
	float WallsChance = GenerationSettings.WallsChance; // Copy to decrease chance after each failed generation
	int32 BoxesChance = GenerationSettings.BoxesChance;
	TMap<FCell, EActorType> ActorsToSpawn;
	int32 Counter = 0;
	bool bFoundPath = false;
	while (WallsChance > KINDA_SMALL_NUMBER // exit if there is no chance to generate level
	       && !bFoundPath)                  // exit if level was generated
	{
		// Set Loop Locals
		FCells LDraggedCells{DraggedCells};
		FCells LCellsToFind{DraggedItems};
		TMap<FCell, EActorType> LActorsToSpawn;

		// Locals
		const FIntVector MapScale(GetActorScale3D());
		const FIntVector MapHalfScale(MapScale / 2);
		FCells WallsToSpawn;

		// --- Part 0: Cells filling ---

		for (int32 Y = 0; Y <= MapHalfScale.Y; ++Y) // Strings
		{
			for (int32 X = 0; X <= MapHalfScale.X; ++X) // Columns
			{
				const bool bIsSafeA = X == 0 && Y == 1;
				const bool bIsSafeB = X == 1 && Y == 0;
				const bool IsSafeZone = bIsSafeA || bIsSafeB;
				FCell CellIt = LocalGridCellsInternal[MapScale.X * Y + X];

				// --- Part 0: Actors random filling to the ArrayToGenerate._ ---

				// In case all next conditions will be false
				EActorType ActorTypeToSpawn = EAT::None;

				// Player condition
				if (X == 0 && Y == 0) // is first corner
				{
					ActorTypeToSpawn = EAT::Player;
				}

				// Wall condition
				if (ActorTypeToSpawn == EAT::None                           // all previous conditions are false
				    && !IsSafeZone && FMath::RandHelper(100) < WallsChance) // chance of walls
				{
					ActorTypeToSpawn = EAT::Wall;
				}

				// Box condition
				if (ActorTypeToSpawn == EAT::None                           // all previous conditions are false
				    && !IsSafeZone && FMath::RandHelper(100) < BoxesChance) // Chance of boxes
				{
					ActorTypeToSpawn = EAT::Box;
				}

				if (ActorTypeToSpawn == EAT::None) // There is no types to spawn
				{
					continue;
				}

				// 0.1) Array symmetrization
				const int32 Xs = MapScale.X - 1 - X, Ys = MapScale.Y - 1 - Y; // Symmetrized cell position
				for (int32 I = 0; I < 4; ++I)                                 // 4 sides of symmetry
				{
					if (I > 0) // the 0 index is always current CellIt, otherwise needs to find symmetry
					{
						int32 Xi = X, Yi = Y; // Keeping the current coordinates
						switch (I)
						{
							case 1: // (X1 = Xs; Y1 = Y)
								Xi = Xs;
								break;
							case 2: // (X2 = X; Y2 = Ys)
								Yi = Ys;
								break;
							case 3: // (X3 = Xs; Y3 = Ys)
								Xi = Xs;
								Yi = Ys;
								break;
							default:
								break;
						}

						CellIt = LocalGridCellsInternal[MapScale.X * Yi + Xi];
					}

					if (!LDraggedCells.Contains(CellIt)) // the cell is free
					{
						LActorsToSpawn.Emplace(CellIt, ActorTypeToSpawn);
						if (ActorTypeToSpawn == EAT::Wall)
						{
							WallsToSpawn.Emplace(CellIt);
						}
						else if (ActorTypeToSpawn == EAT::Player
						         || ActorTypeToSpawn == EAT::Box)
						{
							LCellsToFind.Emplace(CellIt);
						}
					}
				} // Symmetry iterations
			}     // X iterations
		}         // Y iterations

		// --- Part 1 : Checking if there is a path to the bottom and side edges. If not, go to the 0 step._ ---

		const FCells PathBreakers = WallsToSpawn.Union(DraggedWalls);
		bFoundPath = DoesPathExistToCells(LCellsToFind, PathBreakers);

		// Go to the step 0 if don't found
		if (!bFoundPath)
		{
			++Counter;
			WallsChance -= WallsChance * 0.01f; // decrease local chance of walls to avoid forever loop
			continue;
		}

		// Paths were found, exit-loop condition (bFoundPath == true)
		ActorsToSpawn = LActorsToSpawn;
	}

	// --- Part 2: Spawning ---

	SpawnActorsByTypes(ActorsToSpawn);
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

// Is called on client to broadcast On Generated Level Actors delegate
void AGeneratedMap::OnRep_MapComponents()
{
	// Array of level actors is just replicated, try to broadcast On Generated Level Actors delegate
	if (OnGeneratedLevelActors.IsBound()
	    && AMyGameStateBase::GetCurrentGameState() != ECGS::InGame
	    && !MapComponentsInternal.ContainsByPredicate([](const FMapComponentSpec& It) { return !It.IsValid(); }))
	{
		// It is not regular match, probably is Menu state or game is currently starting (3-2-1)
		// and array contains only valid specs (all actors are spawned and replicated)
		OnGeneratedLevelActors.Broadcast();
	}
}

/* ---------------------------------------------------
 *					Editor development
 * --------------------------------------------------- */

// The dragged version of the Add To Grid function to add the dragged actor on the level
void AGeneratedMap::AddToGridDragged(UMapComponent* AddedComponent)
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
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

	FPoolObjectData ObjectData(ComponentOwner);
	ObjectData.bIsActive = true;
	UPoolManagerSubsystem::Get().RegisterObjectInPool(ObjectData);
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor
void AGeneratedMap::SetNearestCellDragged(const UMapComponent* MapComponent, const FCell& NewCell)
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!FEditorUtilsLibrary::IsEditorNotPieWorld()
	    || !MapComponent
	    || !IsDraggedMapComponent(MapComponent)
	    || NewCell.IsInvalidCell())
	{
		return;
	}

	const FCell& CurrentCell = MapComponent->GetCell();
	if (CurrentCell == NewCell)
	{
		return;
	}

	DraggedCellsInternal.Remove(CurrentCell);
	DraggedCellsInternal.Emplace(NewCell, MapComponent->GetActorType());
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