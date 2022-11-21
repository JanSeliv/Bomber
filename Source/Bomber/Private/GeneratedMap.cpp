// Copyright (c) Yevhenii Selivanov.

#include "GeneratedMap.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/DataAssetsContainer.h"
#include "UtilityLibraries/SingletonLibrary.h"
#include "LevelActors/BombActor.h"
#include "PoolManager.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
//---
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/LevelStreaming.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
//---
#if WITH_EDITOR
#include "EditorUtilsLibrary.h"
#include "EditorLevelUtils.h"
#include "EditorUtilityLibrary.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Engine/LevelStreamingDynamic.h"
#endif

/* ---------------------------------------------------
 *		Level map public functions
 * --------------------------------------------------- */

// Returns the generated map data asset
const UGeneratedMapDataAsset& UGeneratedMapDataAsset::Get()
{
	const UGeneratedMapDataAsset* GeneratedMapDataAsset = UDataAssetsContainer::GetLevelsDataAsset();
	checkf(GeneratedMapDataAsset, TEXT("The Generated Map Data Asset is not valid"))
	return *GeneratedMapDataAsset;
}

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this actor to call Tick() every time to update characters locations
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Replicate an actor
	bReplicates = true;
	NetUpdateFrequency = 10.f;
	bAlwaysRelevant = true;

#if WITH_EDITOR	 //[Editor]
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;
#endif	//WITH_EDITOR [Editor]

	// Initialize the Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	static const FVector DefaultRelativeScale(9.F, 9.F, 1.F);
	RootComponent->SetRelativeScale3D_Direct(DefaultRelativeScale);

	// Find blueprint class of the background
	CollisionComponentInternal = CreateDefaultSubobject<UChildActorComponent>(TEXT("Collision Component"));
	CollisionComponentInternal->SetupAttachment(RootComponent);

	// Default camera class
	CameraComponentInternal = CreateDefaultSubobject<UMyCameraComponent>(TEXT("Camera Component"));
	CameraComponentInternal->SetupAttachment(RootComponent);

	// Initialize the Pool Manager
	PoolManagerInternal = CreateDefaultSubobject<UPoolManager>(TEXT("Pool Manger"));
}

// Returns the generated map
AGeneratedMap& AGeneratedMap::Get()
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	checkf(LevelMap, TEXT("The Level Map is not valid"));
	return *LevelMap;
}

// Initialize this Level Map actor, could be called multiple times
void AGeneratedMap::ConstructLevelMap(const FTransform& Transform)
{
	if (OnLevelMapWantsReconstruct.IsBound())
	{
		OnLevelMapWantsReconstruct.Broadcast(Transform);
	}

	OnConstructionLevelMap(Transform);
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
		IntersectCellsByTypes(Walls, TO_FLAG(EAT::Wall)); // just finding the walls on the map
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
	const int32 C0 = GridCellsInternal.IndexOfByPredicate([&Cell](const FCell& InCell) { return InCell == Cell; });
	if (C0 == INDEX_NONE) // if index was found and cell is contained in the array
	{
		return;
	}

	// ----- A path without obstacles -----
	FCells Obstacles;
	const bool bBreakOnObstacles = !bIsAnyPath && Pathfinder != EPathType::Explosion;
	if (bBreakOnObstacles) // if is the request to find the path without Bombs/Boxes
	{
		IntersectCellsByTypes(Obstacles, TO_FLAG(EAT::Bomb | EAT::Box));
	}

	// ----- Secure: a path without players -----
	FCells PlayersCells;
	const bool bBreakOnPlayers = Pathfinder == EPathType::Secure;
	if (bBreakOnPlayers) // if is the request to find the path without players cells.
	{
		IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));
	}

	// ----- A path without explosions -----
	FCells DangerousCells = AdditionalDangerousCells;
	const bool bBreakOnExplosions = Pathfinder == EPathType::Safe || Pathfinder == EPathType::Secure;
	if (bBreakOnExplosions) // if is the request to find the path without explosions.
	{
		FMapComponents BombsMapComponents;
		GetMapComponents(BombsMapComponents, TO_FLAG(EAT::Bomb));
		if (BombsMapComponents.Num() > 0)
		{
			for (const UMapComponent* MapComponentIt : BombsMapComponents)
			{
				const ABombActor* BombOwner = MapComponentIt ? MapComponentIt->GetOwner<ABombActor>() : nullptr;
				if (IS_VALID(BombOwner)) // is valid and is not transient
				{
					DangerousCells = DangerousCells.Union(BombOwner->GetExplosionCells());
				}
			}
		}
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
			const FCell CellDirection = SideMultiplier * (bIsY ? FVector::BackwardVector : FVector::RightVector);
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
				    || !GridCellsInternal.IsValidIndex(FoundIndex))                      // is not in range
				{
					break; // to the next side
				}

				const FCell FoundCell = GridCellsInternal[FoundIndex];

				if (bBreakOnWalls && Walls.Contains(FoundCell)                   // cell contains a wall
				    || bBreakOnObstacles && Obstacles.Contains(FoundCell)        // cell contains an obstacle (Bombs/Boxes)
				    || bBreakOnPlayers && PlayersCells.Contains(FoundCell)       // cell contains a player
				    || bBreakOnExplosions && DangerousCells.Contains(FoundCell)) // cell contains an explosion
				{
					break; // to the next side
				}

				OutCells.Emplace(FoundCell);
			} // Cells iterating
		}     // Each side iterating: -1(Left|Down) and 1(Right|Up)
	}         // Each direction iterating: 0(X-raw) and 1(Y-column)
}

// Returns true if any player is able to reach all specified cells by any any path
bool AGeneratedMap::DoesPathExistToCells(const FCells& CellsToFind, const FCells& OptionalPathBreakers/* = FCell::EmptyCells*/)
{
	FCells InOutSideCells = OptionalPathBreakers;
	if (OptionalPathBreakers.IsEmpty())
	{
		// Include walls to prevent finding way through their cells
		InOutSideCells = UCellsUtilsLibrary::GetAllCellsWithActors(TO_FLAG(EAT::Wall));
	}

	// Contains all cells need to find their side cells
	check(GridCellsInternal.IsValidIndex(0));
	FCells CellsToIterate{GridCellsInternal[0]};

	FCells FoundCells = FCell::EmptyCells;
	while (CellsToIterate.Num())
	{
		// Cache all previous side cells
		const FCells PrevSideCells = InOutSideCells;

		for (const FCell& CellIt : CellsToIterate)
		{
			constexpr float MaxInteger = TNumericLimits<int32>::Max();
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

// Spawns level actor on the Level Map by the specified type
AActor* AGeneratedMap::SpawnActorByType(EActorType Type, const FCell& Cell)
{
	if (!HasAuthority()
	    || !ensureMsgf(PoolManagerInternal, TEXT("ASSERT: 'PoolManagerInternal' is not valid"))
	    || UCellsUtilsLibrary::IsCellHasAnyMatchingActor(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
	    || Type == EAT::None)                                                         // nothing to spawn
	{
		return nullptr;
	}

	const UClass* ClassToSpawn = UDataAssetsContainer::GetActorClassByType(Type);
	AActor* SpawnedActor = PoolManagerInternal->TakeFromPool<AActor>(FTransform(Cell), ClassToSpawn);
	if (!ensureMsgf(SpawnedActor, TEXT("ASSERT: 'SpawnedActor' is not valid")))
	{
		return nullptr;
	}

	SpawnedActor->SetOwner(this);
	return SpawnedActor;
}

// The function that places the actor on the Level Map, attaches it and writes this actor to the GridArray_
void AGeneratedMap::AddToGrid(UMapComponent* AddedComponent)
{
	AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!HasAuthority()
	    || !IS_VALID(ComponentOwner) // the component's owner is not valid or is transient
	    || !ComponentOwner->HasAuthority()
	    || !ensureMsgf(PoolManagerInternal, TEXT("ASSERT: 'PoolManagerInternal' is not valid")))
	{
		return;
	}

	const FCell& Cell = AddedComponent->GetCell();
	if (!ensureMsgf(Cell.IsValid(), TEXT("ASSERT: 'Cell' is zero")))
	{
		return;
	}

	AddToGridDragged(AddedComponent);

	const EActorType ActorType = AddedComponent->GetActorType();

	if (!MapComponentsInternal.Contains(AddedComponent)) // is not contains in the array
	{
		MapComponentsInternal.Emplace(AddedComponent);
		if (ActorType == EAT::Player) // Is a player
		{
			PlayersNumInternal++;
		}
	}

	// begin: find transform
	FRotator ActorRotation{GetCachedTransform().GetRotation()};
	if (!(TO_FLAG(ActorType) & TO_FLAG(EAT::Item | EAT::Player)))
	{
		// Random rotate if is not item and not player
		static constexpr float RotationMultiplier = 90.f;
		static constexpr int32 MinRange = 1;
		static constexpr int32 MaxRange = 4;
		ActorRotation.Yaw += FMath::RandRange(MinRange, MaxRange) * RotationMultiplier;
	}
	static constexpr float HeightAdditive = 100.f;
	const FVector ActorLocation{Cell.Location.X, Cell.Location.Y, Cell.Location.Z + HeightAdditive};
	// end

	// Attach to the Level Map actor
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
	if (!GridCellsInternal.Num()                       // nothing to intersect
	    || !bIntersectAllIfEmpty && !InOutCells.Num()) // should not intersect with all existed cells but the specified array is empty
	{
		return;
	}

	if (!ActorsTypesBitmask)
	{
		// Find all empty grid cell locations where non of actors are present
		const FCells AllEmptyCells = FCells(
			GridCellsInternal.FilterByPredicate([&MapComponents = MapComponentsInternal](const FCell& CellIt)
			{
				return !MapComponents.ContainsByPredicate([&CellIt](const UMapComponent* MapComponentIt)
				{
					return MapComponentIt && MapComponentIt->GetCell() == CellIt;
				});
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
	GetMapComponents(BitmaskedComponents, ActorsTypesBitmask);
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
	for (int32 i = MapComponentsInternal.Num() - 1; i >= 0; --i)
	{
		if (!MapComponentsInternal.IsValidIndex(i)) // the element already was removed
		{
			continue;
		}

		UMapComponent* MapComponentIt = MapComponentsInternal[i];
		const AActor* OwnerIt = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		if (!OwnerIt                                                        // if is null, destroy that object from the array
		    || MapComponentIt && Cells.Contains(MapComponentIt->GetCell())) // the cell is contained on the grid
		{
			// Remove from the array
			// First removing, because after the box destroying the item can be spawned and starts searching for an empty cell
			// MapComponentIt can be invalid here
			DestroyLevelActor(MapComponentIt, DestroyCauser);
		}
	}
	MapComponentsInternal.Shrink();
}

// Removes the specified map component from the MapComponents_ array without an owner destroying
void AGeneratedMap::DestroyLevelActor(UMapComponent* MapComponent, UObject* DestroyCauser/* = nullptr*/)
{
	if (!HasAuthority())
	{
		return;
	}

	AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;

	// Remove from the array (MapComponent can be invalid)
	MapComponentsInternal.Remove(MapComponent);

	if (!ComponentOwner)
	{
		return;
	}

	const bool bIsPlayer = MapComponent->GetActorType() == EAT::Player;
	const bool bIsInGame = AMyGameStateBase::GetCurrentGameState() == ECurrentGameState::InGame;

	if (bIsInGame
	    && MapComponent->IsUndestroyable())
	{
		// Do not destroy actor during the game session if required
		return;
	}

	if (bIsPlayer)
	{
		--PlayersNumInternal;

		if (bIsInGame
		    && OnAnyCharacterDestroyed.IsBound())
		{
			OnAnyCharacterDestroyed.Broadcast();
		}
	}

	MapComponent->OnDeactivated(DestroyCauser);

	// Deactivate the iterated owner
	if (PoolManagerInternal)
	{
		PoolManagerInternal->ReturnToPool(ComponentOwner);
	}

	DestroyLevelActorDragged(MapComponent);
}

// Finds the nearest cell pointer to the specified Map Component
void AGeneratedMap::SetNearestCell(UMapComponent* MapComponent)
{
	const AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!IS_VALID(ComponentOwner)
	    || !HasAuthority()
	    || !ComponentOwner->HasAuthority())
	{
		return;
	}

	// ----- Part 0: Locals -----
	FCell FoundCell = FCell::InvalidCell;
	const FCell OwnerCell(ComponentOwner->GetActorLocation()); // The owner location
	// Check if the owner already standing on:
	FCells InitialCells({OwnerCell,                                                                                        // 0: exactly the current his cell
	                     FCell(OwnerCell.RotateAngleAxis(-1.F).Location.GridSnap(FCell::CellSize)).RotateAngleAxis(1.F)}); // 1: within the radius of one cell

	FCells CellsToIterate(GridCellsInternal.FilterByPredicate([InitialCells](FCell Cell)
	{
		return InitialCells.Contains(Cell);
	}));

	const int32 InitialCellsNum = CellsToIterate.Num(); // The number of initial cells

	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(~EAT::Player)); //AT::Bomb | AT::Item | AT::Wall | AT::Box
	NonEmptyCells.Remove(MapComponent->GetCell());

	// Pre gameplay locals to find a nearest cell
	const bool bHasNotBegunPlay = !HasActorBegunPlay(); // the game was not started
	static constexpr float MaxFloat = TNumericLimits<float>::Max();
	float LastFoundEditorLen = MaxFloat;
	if (bHasNotBegunPlay)
	{
		CellsToIterate.Append(GridCellsInternal); // union of two sets(initials+all) for finding a nearest cell
	}

	// ----- Part 1:  Cells iteration

	int32 Counter = INDEX_NONE;
	for (const FCell& CellIt : CellsToIterate)
	{
		Counter++;

		if (NonEmptyCells.Contains(CellIt)) // the cell is not free from other level actors
		{
			continue;
		}

		// if the cell was found among initial cells without searching a nearest
		if (Counter < InitialCellsNum              // is the initial cell
		    && GridCellsInternal.Contains(CellIt)) // is contained on the grid
		{
			FoundCell = CellIt;
			break;
		}

		//	Finding the nearest cell before starts the game
		if (bHasNotBegunPlay               // the game was not started
		    && Counter >= InitialCellsNum) // if iterated cell is not initial
		{
			const float EditorLenIt = FCell::Distance<float>(OwnerCell, CellIt);
			if (EditorLenIt < LastFoundEditorLen) // Distance closer
			{
				LastFoundEditorLen = EditorLenIt;
				FoundCell = CellIt;
			}
		}
	} //[Cells  Iteration]

	// Checks the cell is contained in the grid and free from other level actors.
	if (FoundCell.IsInvalidCell()) // can be invalid if nothing was found, check to avoid such rewriting
	{
		return;
	}

	SetNearestCellDragged(MapComponent, FoundCell);

	MapComponent->SetCell(FoundCell);
}

// Change level by type
void AGeneratedMap::SetLevelType(ELevelType NewLevelType)
{
	if (!HasAuthority())
	{
		return;
	}

	LevelTypeInternal = NewLevelType;
	ApplyLevelType();
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

/* ---------------------------------------------------
 *		Level map protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned
void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConstructLevelMap(Transform);
}

// Initialize this Level Map actor, could be called multiple times
void AGeneratedMap::OnConstructionLevelMap(const FTransform& Transform)
{
	if (IS_TRANSIENT(this)) // the level map is transient
	{
		return;
	}

#if WITH_EDITOR // [GEditor]
	USingletonLibrary::SetLevelMap(this);
	if (GEditor // Can be bound before editor is loaded
	    && !USingletonLibrary::GOnAnyDataAssetChanged.IsBoundToObject(this))
	{
		// Should be bind in construction in a case of object reconstructing after blueprint compile
		USingletonLibrary::GOnAnyDataAssetChanged.AddUObject(this, &ThisClass::RerunConstructionScripts);
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

	// Add all level actors to the pool manager
	InitPoolManager();

	// Align transform and build cells
	TransformLevelMap(Transform);

#if WITH_EDITOR // [Editor-Standalone]
	if (USingletonLibrary::HasWorldBegunPlay()
	    && PoolManagerInternal)
	{
		// Level actors are spawned differently on client for unsaved level if run without RunInderOneProcess or Standalone
		// so destroy from pool all unsaved level actors to avoid it being unsynced on clients
		auto IsDirtyPredicate = [](const UObject* PoolObject) -> bool
		{
			return PoolObject && !PoolObject->HasAllFlags(RF_WasLoaded | RF_LoadCompleted);
		};

		PoolManagerInternal->EmptyAllByPredicate(IsDirtyPredicate);
	}
#endif // WITH_EDITOR // [Editor-Standalone]

	// Actors generation
	GenerateLevelActors();

	// Update level stream
	ApplyLevelType();

	// Update camera position
	if (CameraComponentInternal)
	{
		CameraComponentInternal->UpdateMaxHeight();
		CameraComponentInternal->UpdateLocation();
	}
}

// Called right before components are initialized, only called during gameplay
void AGeneratedMap::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register level actors for game features
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// This is called only in the gameplay before calling begin play to generate level actors
void AGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (IS_TRANSIENT(this)) // the level map is transient
	{
		return;
	}

	// Update the gameplay LevelMap reference in the singleton library
	USingletonLibrary::SetLevelMap(this);

	ConstructLevelMap(GetActorTransform());

	if (HasAuthority())
	{
		// Listen states
		if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
		{
			MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
		}
	}
}

// Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending
void AGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this)
	    && HasAuthority())
	{
		// Destroy level actors
		if (PoolManagerInternal)
		{
			PoolManagerInternal->EmptyAllPools();
		}

		// Destroy level actors in internal arrays
		const int32 MapComponentsNum = MapComponentsInternal.Num();
		for (int32 Index = MapComponentsNum - 1; Index >= 0; --Index)
		{
			UMapComponent* MapComponentIt = MapComponentsInternal.IsValidIndex(Index) ? MapComponentsInternal[Index] : nullptr;
			DestroyLevelActor(MapComponentIt);
		}

#if WITH_EDITOR // [IsEditorNotPieWorld]
		if (UEditorUtilsLibrary::IsEditorNotPieWorld())
		{
			// Remove editor bound delegates
			USingletonLibrary::GOnAnyDataAssetChanged.RemoveAll(this);
			FEditorDelegates::OnMapOpened.RemoveAll(this);
		}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::Destroyed();
}

// Returns properties that are replicated for the lifetime of the actor channel
void AGeneratedMap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MapComponentsInternal);
	DOREPLIFETIME(ThisClass, PlayersNumInternal);
	DOREPLIFETIME(ThisClass, LevelTypeInternal);
	DOREPLIFETIME(ThisClass, bIsGameRunningInternal);
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	if (!ensureMsgf(GridCellsInternal.Num() > 0, TEXT("Is no cells for the actors generation"))
	    || !HasAuthority())
	{
		return;
	}

	// Destroy all editor-only non-PIE actors
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::All));
	DestroyLevelActorsOnCells(NonEmptyCells);
	PlayersNumInternal = 0;

	// Calls before generation preview actors to updating of all dragged to the Level Map actors
	FCells DraggedCells, DraggedWalls, DraggedItems;
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

	const UGeneratedMapDataAsset& LevelsDataAsset = UGeneratedMapDataAsset::Get();
	float WallsChance = LevelsDataAsset.GetWallsChance(); // Copy to decrease chance after each failed generation
	int32 BoxesChance = LevelsDataAsset.GetBoxesChance();
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
		const FIntVector MapScale(GetCachedTransform().GetScale3D());
		const FIntVector MapHalfScale(MapScale / 2);
		FCells WallsToSpawn;

		// --- Part 0: Cells filling ---

		for (int32 Y = 0; Y <= MapHalfScale.Y; ++Y) // Strings
		{
			for (int32 X = 0; X <= MapHalfScale.X; ++X) // Columns
			{
				const bool IsSafeZone = X == 0 && Y == 1 || X == 1 && Y == 0;
				FCell CellIt = GridCellsInternal[MapScale.X * Y + X];

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

						CellIt = GridCellsInternal[MapScale.X * Yi + Xi];
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

	for (const TTuple<FCell, EActorType>& It : ActorsToSpawn)
	{
		SpawnActorByType(It.Value, It.Key);
	}
}

//  Map components getter.
void AGeneratedMap::GetMapComponents(FMapComponents& OutBitmaskedComponents, int32 ActorsTypesBitmask) const
{
	if (!MapComponentsInternal.Num())
	{
		return;
	}

	for (UMapComponent* MapComponentIt : MapComponentsInternal)
	{
		if (MapComponentIt
		    && EnumHasAnyFlags(MapComponentIt->GetActorType(), TO_ENUM(EActorType, ActorsTypesBitmask)))
		{
			OutBitmaskedComponents.Emplace(MapComponentIt);
		}
	}
}

// Listen game states to generate level actors
void AGeneratedMap::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (CurrentGameState)
	{
		case ECurrentGameState::Menu:
		{
			// on returning to menu case
			GenerateLevelActors();
			bIsGameRunningInternal = false;
			break;
		}

		case ECurrentGameState::GameStarting:
		{
			if (bIsGameRunningInternal)
			{
				// on reset pressed case
				GenerateLevelActors();
			}

			bIsGameRunningInternal = true;
			AdditionalDangerousCells.Reset();
			break;
		}

		default:
			break;
	}
}

// Align transform and build cells
void AGeneratedMap::TransformLevelMap(const FTransform& Transform)
{
	CachedTransformInternal = FTransform::Identity;

	// Align the Transform
	const FVector NewLocation = UGeneratedMapDataAsset::Get().IsLockedOnZero()
		                            ? FVector::ZeroVector
		                            : Transform.GetLocation().GridSnap(FCell::CellSize);
	CachedTransformInternal.SetLocation(NewLocation);

	const FRotator NewRotation(0.f, Transform.GetRotation().Rotator().Yaw, 0.f);
	CachedTransformInternal.SetRotation(NewRotation.Quaternion());

	FIntVector MapScale(Transform.GetScale3D());
	MapScale.Z = 1;          //Height must be 1
	if (MapScale.X % 2 != 1) // Length must be unpaired
	{
		MapScale.X += 1;
	}
	if (MapScale.Y % 2 != 1) // Weight must be unpaired
	{
		MapScale.Y += 1;
	}
	const FVector NewScale3D(MapScale);
	CachedTransformInternal.SetScale3D(NewScale3D);

	SetActorTransform(CachedTransformInternal);

	GridCellsInternal.Empty();
	GridCellsInternal.Reserve(MapScale.X * MapScale.Y);

	// Loopy cell-filling of the grid array
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			FVector FoundVector(X, Y, 0.f);
			// Calculate a length of iteration cell
			FoundVector *= FCell::CellSize;
			// Locate the cell relative to the Level Map
			FoundVector += NewLocation;
			// Subtract the deviation from the center
			FoundVector -= (NewScale3D / 2) * FCell::CellSize;
			// Snap to the cell
			FoundVector = FoundVector.GridSnap(FCell::CellSize);
			// Cell was found, add rotated cell to the array
			const FCell FoundCell(FCell(FoundVector).RotateAngleAxis(1.f));
			GridCellsInternal.AddUnique(FoundCell);
		}
	}
}

// Updates current level type
void AGeneratedMap::ApplyLevelType()
{
	UWorld* World = GetWorld();
	TArray<FLevelStreamRow> LevelStreamRows;
	UGeneratedMapDataAsset::Get().GetLevelStreamRows(LevelStreamRows);
	if (!LevelStreamRows.Num()
	    || !World)
	{
		return;
	}

	// Get Level Streaming by Index in LevelStreamingRows, returns if found stream should be visible
	auto GetLevelStreaming = [&LevelStreamRows, NewLevelType = LevelTypeInternal](const int32& Index, FName& OutLevelName) -> bool
	{
		if (!LevelStreamRows.IsValidIndex(Index))
		{
			return false;
		}

		const FLevelStreamRow& LevelStreamRowIt = LevelStreamRows[Index];
		OutLevelName = *LevelStreamRowIt.Level.GetLongPackageName();
		return LevelStreamRowIt.LevelType == NewLevelType;
	};

	// ---- Changing streaming levels in the preview world ----

#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (UEditorUtilsLibrary::IsEditorNotPieWorld())
	{
		if (LevelTypeInternal == ELT::None)
		{
			// the level is not selected, choose persistent
			UEditorLevelUtils::MakeLevelCurrent(World->PersistentLevel, false);
			return;
		}

		// Iterate all levels, load all, show selected, hide others
		UPackage* PersistentLevelPackage = World->PersistentLevel->GetPackage();
		const bool bIsPersistentLevelDirty = PersistentLevelPackage && PersistentLevelPackage->IsDirty();
		bool bClearPersistentLevelDirty = false;
		for (int32 Index = 0; Index < LevelStreamRows.Num(); ++Index)
		{
			FName PackageName(NAME_None);
			const bool bShouldBeVisibleIt = GetLevelStreaming(Index, PackageName);
			if (PackageName.IsNone())
			{
				continue;
			}

			const TSubclassOf<ULevelStreaming>& StreamingClass = bShouldBeVisibleIt ? ULevelStreamingAlwaysLoaded::StaticClass() : ULevelStreamingDynamic::StaticClass();
			const ULevelStreaming* LevelStreamingIt = UGameplayStatics::GetStreamingLevel(World, PackageName);
			if (!LevelStreamingIt) // level is not added to the persistent
			{
				LevelStreamingIt = UEditorLevelUtils::AddLevelToWorld(World, *PackageName.ToString(), StreamingClass);
			}

			// Set visibility
			ULevel* LoadedLevel = LevelStreamingIt ? LevelStreamingIt->GetLoadedLevel() : nullptr;
			if (LoadedLevel)
			{
				UEditorLevelUtils::SetLevelVisibility(LoadedLevel, bShouldBeVisibleIt, false, ELevelVisibilityDirtyMode::DontModify);
				if (bShouldBeVisibleIt)
				{
					// Make the selected level as current (it will make persistent as dirty)
					UEditorLevelUtils::MakeLevelCurrent(LoadedLevel, true);
					bClearPersistentLevelDirty = !bIsPersistentLevelDirty;
				}
			}
		}

		// Reset dirty flag for persistent marked by UEditorLevelUtils if level was not modified before
		if (bClearPersistentLevelDirty
		    && PersistentLevelPackage)
		{
			PersistentLevelPackage->ClearDirtyFlag();
		}

		// The editor stream was overridden, no need to continue
		return;
	}
#endif // [IsEditorNotPieWorld]

	// ---- Changing levels during the game ----

	// show the specified level, hide other levels
	for (int32 Index = 0; Index < LevelStreamRows.Num(); ++Index)
	{
		FName PackageName;
		const bool bShouldBeVisibleIt = GetLevelStreaming(Index, PackageName);
		if (PackageName.IsNone())
		{
			continue;
		}

		ULevelStreaming* LevelStreaming = UGameplayStatics::GetStreamingLevel(World, PackageName);
		if (!ensureMsgf(LevelStreaming, TEXT("ASSERT: 'LevelStreaming' is not valid")))
		{
			continue;
		}

		if (!LevelStreaming->IsLevelLoaded())
		{
			LevelStreaming->OnLevelLoaded.AddUniqueDynamic(this, &ThisClass::ApplyLevelType);

			FLatentActionInfo LatentInfo;
			LatentInfo.UUID = Index;
			constexpr bool bMakeVisibleAfterLoad = false;
			constexpr bool bShouldBlockOnLoad = false;
			UGameplayStatics::LoadStreamLevel(World, PackageName, bMakeVisibleAfterLoad, bShouldBlockOnLoad, LatentInfo);
		}
		else
		{
			LevelStreaming->SetShouldBeVisible(bShouldBeVisibleIt);
		}
	}

	if (OnSetNewLevelType.IsBound())
	{
		OnSetNewLevelType.Broadcast(LevelTypeInternal);
	}

	// Once level is loading, prepare him
	for (UMapComponent* MapComponentIt : MapComponentsInternal)
	{
		if (MapComponentIt)
		{
			MapComponentIt->ConstructOwnerActor();
		}
	}
}

// Is called on client and server to load new level
void AGeneratedMap::OnRep_LevelType()
{
	ApplyLevelType();
}

// Find and add all level actors to allow the Pool Manager to handle all of them
void AGeneratedMap::InitPoolManager()
{
	if (!HasAuthority()
	    || !PoolManagerInternal)
	{
		return;
	}

	TArray<AActor*> Owners;

	// Get first all attached level actors 
	GetAttachedActors(/*out*/Owners);

	// Get players separately since they are not attached to the level 
	FMapComponents PlayerMapComponents;
	GetMapComponents(PlayerMapComponents, TO_FLAG(EAT::Player));
	for (const UMapComponent* PlayerMapComponentIt : PlayerMapComponents)
	{
		if (PlayerMapComponentIt)
		{
			Owners.AddUnique(PlayerMapComponentIt->GetOwner());
		}
	}

	// Add all found level actors to the pool manager
	for (AActor* OwnerIt : Owners)
	{
		if (!USingletonLibrary::IsLevelActor(OwnerIt))
		{
			continue;
		}

		PoolManagerInternal->AddToPool(OwnerIt);
	}
}

/* ---------------------------------------------------
 *					Editor development
 * --------------------------------------------------- */

#if WITH_EDITOR	 // [GEditor]PostLoad();
// Do any object-specific cleanup required immediately after loading an object. This is not called for newly-created objects
void AGeneratedMap::PostLoad()
{
	Super::PostLoad();

	if (!GEditor) // is bound before editor is loaded to update streams
	{
		return;
	}

	// Update streaming level on editor opening
	TWeakObjectPtr<AGeneratedMap> WeakLevelMap(this);
	auto UpdateLevelType = [WeakLevelMap](const FString&, bool)
	{
		if (AGeneratedMap* LevelMap = WeakLevelMap.Get())
		{
			FEditorDelegates::OnMapOpened.RemoveAll(LevelMap);
			LevelMap->SetLevelType(LevelMap->LevelTypeInternal);
		}
	};

	if (!FEditorDelegates::OnMapOpened.IsBoundToObject(this))
	{
		FEditorDelegates::OnMapOpened.AddWeakLambda(this, UpdateLevelType);
	}
}
#endif	// WITH_EDITOR [GEditor]PostLoad();

// The dragged version of the Add To Grid function to add the dragged actor on the level
void AGeneratedMap::AddToGridDragged(UMapComponent* AddedComponent)
{
#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld())
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
	// Each similar not dragged actor that is generated by Level Map is preview actor, so is not dragged.

	// Move dragged actor from a sublevel to the Main level
	ULevel* MainLevel = GetLevel();
	const ULevel* OwnerLevel = ComponentOwner->GetLevel();
	if (MainLevel != OwnerLevel)
	{
		const FTransform OwnerTransform = ComponentOwner->GetActorTransform();

		UEditorLevelUtils::MoveSelectedActorsToLevel(MainLevel);

		const TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
		AActor* CopiedActor = SelectedActors.IsValidIndex(0) ? SelectedActors[0] : nullptr;
		UMapComponent* CopiedComponent = CopiedActor ? UMapComponent::GetMapComponent(CopiedActor) : nullptr;
		if (CopiedComponent
		    && CopiedComponent != AddedComponent)
		{
			CopiedActor->SetActorTransform(OwnerTransform);
			SetNearestCell(CopiedComponent);
			AddToGrid(CopiedComponent);
		}

		return; // new actor will be copied, this one no need to add
	}

	if (!DraggedCellsInternal.Contains(AddedComponent->GetCell()))
	{
		DraggedCellsInternal.Emplace(AddedComponent->GetCell(), AddedComponent->GetActorType());
	}

	if (PoolManagerInternal)
	{
		PoolManagerInternal->AddToPool(ComponentOwner);
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// The dragged version of the Set Nearest Cell function to find closest cell for the dragged level actor
void AGeneratedMap::SetNearestCellDragged(const UMapComponent* MapComponent, const FCell& NewCell)
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld()
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
	if (!UEditorUtilsLibrary::IsEditorNotPieWorld()
	    || !MapComponent
	    || IS_VALID(MapComponent->GetOwner()))
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
