// Copyright 2021 Yevhenii Selivanov.

#include "GeneratedMap.h"
//---
#include "Bomber.h"
#include "Structures/Cell.h"
#include "Components/MapComponent.h"
#include "Components/MyCameraComponent.h"
#include "GameFramework/MyGameStateBase.h"
#include "Globals/SingletonLibrary.h"
#include "LevelActors/BombActor.h"
//---
#include "Engine/LevelStreaming.h"
#include "Math/UnrealMathUtility.h"
//---
#if WITH_EDITOR
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
	const UGeneratedMapDataAsset* GeneratedMapDataAsset = USingletonLibrary::GetLevelsDataAsset();
	checkf(GeneratedMapDataAsset, TEXT("The Generated Map Data Asset is not valid"))
	return *GeneratedMapDataAsset;
}

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this actor to call Tick() every time to update characters locations
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// setup replication
	bReplicates = true;

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
	CameraComponentInternal = CreateDefaultSubobject<UMyCameraComponent>(TEXT("CameraComponent"));
	CameraComponentInternal->SetupAttachment(RootComponent);
}

// Returns the generated map
AGeneratedMap& AGeneratedMap::Get()
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	checkf(LevelMap, TEXT("The Level Map is not valid"));
	return *LevelMap;
}

// Getting an array of cells by four sides of an input center cell and type of breaks
void AGeneratedMap::GetSidesCells(
	FCells& OutCells,
	const FCell& Cell,
	EPathType Pathfinder,
	int32 SideLength,
	bool bBreakInputCells) const
{
	// ----- Walls definition -----
	FCells Walls;
	if (OutCells.Num() == 0)
	{
		IntersectCellsByTypes(Walls, TO_FLAG(EAT::Wall)); // just finding the walls on the map
	}
	else if (bBreakInputCells) // specified OutCells is not empty, these cells break lines as the Wall behavior
	{
		Walls = OutCells; // these cells break lines as the Wall behavior, don't empty specified array
	}
	else //OutCells > 0 && !bBreakInputCells
	{
		OutCells.Empty(); // should empty array in order to return only sides cells
	}

	// the index of the specified cell
	const int32 C0 = GridCellsInternal.IndexOfByPredicate([Cell](const FCell& InCell) { return InCell == Cell; });
	if (C0 == INDEX_NONE) // if index was found and cell is contained in the array
	{
		return;
	}

	// ----- A path without obstacles -----
	FCells Obstacles;
	const bool bWithoutObstacles = Pathfinder != EPathType::Explosion;
	if (bWithoutObstacles) // if is the request to find the path without Bombs/Boxes
	{
		IntersectCellsByTypes(Obstacles, TO_FLAG(EAT::Bomb | EAT::Box));
	}

	// ----- Secure: a path without players -----
	FCells PlayersCells;
	const bool bWithoutPlayers = Pathfinder == EPathType::Secure;
	if (bWithoutPlayers) // if is the request to find the path without players cells.
	{
		IntersectCellsByTypes(PlayersCells, TO_FLAG(EAT::Player));
	}

	// ----- A path without explosions -----
	FCells DangerousCells;
	const bool bWithoutExplosions = Pathfinder == EPathType::Safe || Pathfinder == EPathType::Secure;
	if (bWithoutExplosions) // if is the request to find the path without explosions.
	{
		FMapComponents BombsMapComponents;
		GetMapComponents(BombsMapComponents, TO_FLAG(EAT::Bomb));
		if (BombsMapComponents.Num() > 0)
		{
			for (const UMapComponent* const& MapComponentIt : BombsMapComponents)
			{
				const auto BombOwner = MapComponentIt ? Cast<ABombActor>(MapComponentIt->GetOwner()) : nullptr;
				if (IS_VALID(BombOwner)) // is valid and is not transient
				{
					DangerousCells = DangerousCells.Union(BombOwner->GetExplosionCells());
				}
			}
		}
	}

	// ----- The specified cell adding -----
	if (bWithoutExplosions == false        // can be danger
	    || !DangerousCells.Contains(Cell)) // is not dangerous cell
	{
		OutCells.Emplace(Cell);
	}

	// ----- Cells finding -----
	const int32 MaxWight = GetActorScale3D().X;
	for (int32 bIsY = 0; bIsY <= 1; ++bIsY) // 0(X-raw direction) and 1(Y-column direction)
	{
		const int32 PositionC0 = bIsY ? /*Y-column*/ C0 % MaxWight : C0 / MaxWight /*raw*/;
		for (int32 SideMultiplier = -1; SideMultiplier <= 1; SideMultiplier += 2) // -1(Left|Down) and 1(Right|Up)
		{
			for (int32 i = 1; i <= SideLength; ++i)
			{
				int32 Distance = i * SideMultiplier;
				if (bIsY) { Distance *= MaxWight; }
				const int32 FoundIndex = C0 + Distance;
				if (PositionC0 != (bIsY ? FoundIndex % MaxWight : FoundIndex / MaxWight) // PositionC0 != PositionX
				    || !GridCellsInternal.IsValidIndex(FoundIndex))                      // is not in range
				{
					break; // to the next side
				}

				const FCell FoundCell = GridCellsInternal[FoundIndex];

				if (Walls.Contains(FoundCell)                                    // cell contains a wall
				    || bWithoutObstacles && Obstacles.Contains(FoundCell)        // cell contains an obstacle (Bombs/Boxes)
				    || bWithoutPlayers && PlayersCells.Contains(FoundCell)       // cell contains a player
				    || bWithoutExplosions && DangerousCells.Contains(FoundCell)) // cell contains an explosion
				{
					break; // to the next side
				}

				OutCells.Emplace(FoundCell);
			} // Cells iterating
		}     // Each side iterating: -1(Left|Down) and 1(Right|Up)
	}         // Each direction iterating: 0(X-raw) and 1(Y-column)
}

// Spawns level actor on the Level Map by the specified type
AActor* AGeneratedMap::SpawnActorByType(EActorType Type, const FCell& Cell)
{
	UWorld* World = GetWorld();
	if (!World
	    || ContainsMapComponents(Cell, TO_FLAG(~EAT::Player)) // the free cell was not found
	    || Type == EAT::None)                                 // nothing to spawn
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR
	SpawnParams.bTemporaryEditorActor = USingletonLibrary::IsEditorNotPieWorld();
#endif
	return World->SpawnActor<AActor>(USingletonLibrary::GetActorClassByType(Type), FTransform(Cell.Location), SpawnParams);
}

// The function that places the actor on the Level Map, attaches it and writes this actor to the GridArray_
void AGeneratedMap::AddToGrid(const FCell& Cell, UMapComponent* AddedComponent)
{
	AActor* ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!IS_VALID(ComponentOwner) // the component's owner is not valid or is transient
	    || !ensureMsgf(Cell, TEXT("ASSERT: 'Cell' is zero")))
	{
		return;
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld()
	    && !ComponentOwner->bIsEditorPreviewActor
	    && ComponentOwner->IsSelectedInEditor()
	    && !DraggedComponentsInternal.Contains(AddedComponent))
	{
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
				AddToGrid(CopiedComponent->Cell, CopiedComponent);
			}

			return; // new actor will be copied, this one no need to add
		}

		DraggedComponentsInternal.Emplace(AddedComponent);
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]

	const EActorType ActorType = AddedComponent->GetActorType();

	if (!MapComponentsInternal.Contains(AddedComponent)) // is not contains in the array
	{
		MapComponentsInternal.Emplace(AddedComponent);
		if (ActorType == EAT::Player) // Is a player
		{
			PlayersNumInternal++;
		}
	}

	// Set absolute scale
	if (USceneComponent* OwnerRootComponent = ComponentOwner->GetRootComponent())
	{
		OwnerRootComponent->SetAbsolute(false, false, true);
	}

	// begin: find transform
	FRotator ActorRotation{GetActorRotation()};
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

	// Locate actor on cell
	ComponentOwner->SetActorTransform(FTransform(ActorRotation, ActorLocation, FVector::OneVector));

	// Attach to the Level Map actor
	ComponentOwner->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
}

// The intersection of (OutCells ∩ ActorsTypesBitmask).
void AGeneratedMap::IntersectCellsByTypes(
	FCells& OutCells,
	int32 ActorsTypesBitmask,
	bool bIntersectAllIfEmpty,
	const UMapComponent* ExceptedComponent) const
{
	if (!GridCellsInternal.Num()                     // nothing to intersect
	    || !bIntersectAllIfEmpty && !OutCells.Num()) // should not intersect with all existed cells but the specified array is empty
	{
		return;
	}

	FMapComponents BitmaskedComponents;
	GetMapComponents(BitmaskedComponents, ActorsTypesBitmask);
	if (BitmaskedComponents.Num() == 0)
	{
		OutCells.Empty(); // nothing found, returns empty OutCells array
		return;
	}

	FCells BitmaskedCells;
	for (const UMapComponent* const& MapCompIt : BitmaskedComponents)
	{
		if (MapCompIt
		    && MapCompIt != ExceptedComponent)
		{
			BitmaskedCells.Emplace(MapCompIt->Cell);
		}
	}

	OutCells = OutCells.Num() > 0 ? OutCells.Intersect(BitmaskedCells) : BitmaskedCells;
}

// Checking the containing of the specified cell among owners locations of the Map Components array
bool AGeneratedMap::ContainsMapComponents(const FCell& Cell, int32 ActorsTypesBitmask) const
{
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, ActorsTypesBitmask);
	return NonEmptyCells.Contains(Cell);
}

// Destroy all actors from the set of cells
void AGeneratedMap::DestroyActorsFromMap(const FCells& Cells)
{
	if (!MapComponentsInternal.Num()
	    || !Cells.Num())
	{
		return;
	}

	// Iterate and destroy
	bool OnAnyPlayerDestroyed = false;
	for (int32 i = MapComponentsInternal.Num() - 1; i >= 0; --i)
	{
		if (!MapComponentsInternal.IsValidIndex(i)) // the element already was removed
		{
			continue;
		}

		UMapComponent* MapComponentIt = MapComponentsInternal[i];
		AActor* OwnerIt = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		if (!OwnerIt                                                   // if is null, destroy that object from the array
		    || MapComponentIt && Cells.Contains(MapComponentIt->Cell)) // the cell is contained on the grid
		{
			// Do not destroy actor during the game session if required
			if (MapComponentIt && MapComponentIt->IsUndestroyable()
			    && AMyGameStateBase::GetCurrentGameState(this) == ECurrentGameState::InGame)
			{
				continue;
			}

			// Remove from the array
			// First removing, because after the box destroying the item can be spawned and starts searching for an empty cell
			// MapComponentIt can be invalid here
			DestroyLevelActor(MapComponentIt);

			// Decrement the players number
			if (MapComponentIt && MapComponentIt->GetActorType() == EAT::Player) // Is a player
			{
				--PlayersNumInternal;
				OnAnyPlayerDestroyed = true;
			}
		}
	}
	MapComponentsInternal.Shrink();

	// Update endgame state
	if (OnAnyPlayerDestroyed)
	{
		AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState();
		if (MyGameState
		    && AMyGameStateBase::GetCurrentGameState(this) == ECurrentGameState::InGame)
		{
			MyGameState->OnAnyPlayerDestroyed.Broadcast();
		}
	}
}

// Removes the specified map component from the MapComponents_ array without an owner destroying
void AGeneratedMap::DestroyLevelActor(UMapComponent* MapComponent)
{
	AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	const bool bIsValidOwner = IS_VALID(ComponentOwner);

	// Remove from the array (MapComponent can be invalid)
	MapComponentsInternal.Remove(MapComponent);

	// hide if dragged else destroy
	if (bIsValidOwner                         // forcing destroy dragged actors only when the owner or Generated Map becomes invalid
	    && IsDraggedLevelActor(MapComponent)) // is dragged
	{
		ComponentOwner->SetActorHiddenInGame(true);
		ComponentOwner->SetActorEnableCollision(false);
		return; // is dragged, do not continue destroying that actor
	}

	// Remove if dragged actor (MapComponent can be invalid)
	DraggedComponentsInternal.Remove(MapComponent);

	// Destroy the iterated owner
	if (bIsValidOwner)
	{
		ComponentOwner->SetFlags(RF_Transient); // destroy only once
		ComponentOwner->Destroy();
	}
}

// Finds the nearest cell pointer to the specified Map Component
void AGeneratedMap::SetNearestCell(UMapComponent* MapComponent) const
{
	AActor* ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!IS_VALID(ComponentOwner))
	{
		return;
	}

	// ----- Part 0: Locals -----
	FCell FoundCell;
	const FCell OwnerCell(ComponentOwner->GetActorLocation()); // The owner location
	// Check if the owner already standing on:
	FCells InitialCells({OwnerCell,                                                                                        // 0: exactly the current his cell
	                     FCell(OwnerCell.RotateAngleAxis(-1.F).Location.GridSnap(FCell::CellSize)).RotateAngleAxis(1.F)}); // 1: within the radius of one cell

	TSet<FCell> CellsToIterate(GridCellsInternal.FilterByPredicate([InitialCells](FCell Cell)
	{
		return InitialCells.Contains(Cell);
	}));

	const int32 InitialCellsNum = CellsToIterate.Num();                              // The number of initial cells
	FCells NonEmptyCells;                                                            // all cells of each level actor
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(~EAT::Player), true, MapComponent); //AT::Bomb | AT::Item | AT::Wall | AT::Box

	// Pre gameplay locals to find a nearest cell
	const bool bHasNotBegunPlay = !HasActorBegunPlay(); // the game was not started
	float LastFoundEditorLen = MAX_FLT;
	if (bHasNotBegunPlay)
	{
		CellsToIterate.Append(GridCellsInternal); // union of two sets(initials+all) for finding a nearest cell
	}

	// ----- Part 1:  Cells iteration

	int32 Counter = -1;
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
			const float EditorLenIt = USingletonLibrary::CalculateCellsLength(OwnerCell, CellIt);
			if (EditorLenIt < LastFoundEditorLen) // Distance closer
			{
				LastFoundEditorLen = EditorLenIt;
				FoundCell = CellIt;
			}
		}
	} //[Cells Iteration]

	// Checks the cell is contained in the grid and free from other level actors.
	if (FoundCell) // can be invalid if nothing was found, check to avoid such rewriting
	{
		MapComponent->Cell = FoundCell;
	}
}

// Change level by type
void AGeneratedMap::SetLevelType(ELevelType NewLevelType)
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
	auto GetLevelStreaming = [&LevelStreamRows, NewLevelType](const int32& Index, FName& OutLevelName) -> bool
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
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		if (NewLevelType == ELT::None)
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
			ULevelStreaming* LevelStreamingIt = UGameplayStatics::GetStreamingLevel(World, PackageName);
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

		FLatentActionInfo LatentInfo;
		LatentInfo.UUID = Index;
		ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(World, PackageName);
		if (!StreamingLevel
		    || !StreamingLevel->IsLevelLoaded())
		{
			UGameplayStatics::LoadStreamLevel(World, PackageName, bShouldBeVisibleIt, false, LatentInfo);
		}
		else
		{
			StreamingLevel->SetShouldBeVisible(bShouldBeVisibleIt);
		}
	}

	// skip if the same level type
	if (LevelTypeInternal == NewLevelType)
	{
		return;
	}

	// once level is loading, prepare him
	LevelTypeInternal = NewLevelType;
	for (const TObjectPtr<UMapComponent>& MapComponentIt : MapComponentsInternal)
	{
		if (MapComponentIt)
		{
			MapComponentIt->RerunOwnerConstruction();
		}
	}
}

// Returns cells that currently are chosen to be exploded
void AGeneratedMap::GetDangerousCells(TSet<FCell>& OutCells, const ABombActor* BombInstigator/* = nullptr*/) const
{
	FMapComponents BombsMapComponents;
	GetMapComponents(BombsMapComponents, TO_FLAG(EAT::Bomb));

	FCells DangerousCells;
	for (const UMapComponent* MapComponentIt : BombsMapComponents)
	{
		const auto BombOwner = MapComponentIt ? Cast<ABombActor>(MapComponentIt->GetOwner()) : nullptr;
		if (!IS_VALID(BombOwner)
		    || BombOwner == BombInstigator)
		{
			continue;
		}

		static constexpr float ErrorTolerance = 0.01f;
		const float BombRemainsTime = BombOwner->GetLifeSpan();
		if (!FMath::IsNearlyZero(BombRemainsTime, ErrorTolerance))
		{
			// Skip, is not ready to explode
			continue;
		}

		const FCells ExplosionCells = BombOwner->GetExplosionCells();
		DangerousCells = DangerousCells.Union(ExplosionCells);
	}

	if (BombInstigator)
	{
		// Return only unique cells to explode for specified instigator
		const FCells InstigatorCells = BombInstigator->GetExplosionCells();
		OutCells = InstigatorCells.Difference(DangerousCells);
		return;
	}

	// Return dangerous cells of all bombs
	OutCells = DangerousCells;
}

// Returns the life span for specified cell.
float AGeneratedMap::GetCellLifeSpan(const FCell& Cell, const class ABombActor* BombInstigator/* = nullptr*/) const
{
	float MinLifeSpan = UBombDataAsset::Get().GetLifeSpan();
	if (Cell == FCell::ZeroCell
	    || AMyGameStateBase::GetCurrentGameState(this) != ECurrentGameState::InGame)
	{
		return MinLifeSpan;
	}

	FMapComponents BombsMapComponents;
	GetMapComponents(BombsMapComponents, TO_FLAG(EAT::Bomb));

	for (const UMapComponent* MapComponentIt : BombsMapComponents)
	{
		const auto BombOwner = MapComponentIt ? Cast<ABombActor>(MapComponentIt->GetOwner()) : nullptr;
		if (!IS_VALID(BombOwner)
		    || BombOwner == BombInstigator)
		{
			continue;
		}

		const FCells ExplosionCells = BombOwner->GetExplosionCells();
		const float OwnerLifeSpan = BombOwner->GetLifeSpan();
		if (ExplosionCells.Contains(Cell)
		    && OwnerLifeSpan < MinLifeSpan)
		{
			MinLifeSpan = OwnerLifeSpan;
		}
	}

	return MinLifeSpan;
}

/* ---------------------------------------------------
 *		Level map protected functions
 * --------------------------------------------------- */

// Called when an instance of this class is placed (in editor) or spawned
void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

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
		const TSubclassOf<AActor>& CollisionsAssetClass = UGeneratedMapDataAsset::Get().GetCollisionsAssetClass();
		CollisionComponentInternal->SetChildActorClass(CollisionsAssetClass);
		CollisionComponentInternal->CreateChildActor();
	}

	// Align transform and build cells
	TransformLevelMap(Transform);

	// Update level stream
	SetLevelType(LevelTypeInternal);

	// Update camera position
	if (CameraComponentInternal)
	{
		CameraComponentInternal->UpdateMaxHeight();
		CameraComponentInternal->UpdateLocation();
	}
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

	RerunConstructionScripts();

	// Listen states
	if (AMyGameStateBase* MyGameState = USingletonLibrary::GetMyGameState())
	{
		MyGameState->OnGameStateChanged.AddDynamic(this, &ThisClass::OnGameStateChanged);
	}
}

// Called when is explicitly being destroyed to destroy level actors, not called during level streaming or gameplay ending
void AGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this))
	{
		// Destroy level actors
		FCells NonEmptyCells;
		IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::All));
		DestroyActorsFromMap(NonEmptyCells);

#if WITH_EDITOR // [IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			// Remove editor bound delegates
			USingletonLibrary::GOnAnyDataAssetChanged.RemoveAll(this);
			FEditorDelegates::OnMapOpened.RemoveAll(this);
		}
#endif //WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::Destroyed();
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	if (!ensureMsgf(GridCellsInternal.Num() > 0, TEXT("Is no cells for the actors generation")))
	{
		return;
	}

	// Destroy all editor-only non-PIE actors
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::All));
	DestroyActorsFromMap(NonEmptyCells);
	PlayersNumInternal = 0;

	// Calls before generation preview actors to updating of all dragged to the Level Map actors
	FCells DraggedCells, DraggedWalls, DraggedItems;
	for (const TObjectPtr<UMapComponent>& MapComponentIt : DraggedComponentsInternal)
	{
		AActor* OwnerIt = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		if (!OwnerIt)
		{
			continue;
		}

		// Setup dragged owner
		MapComponentIt->RerunOwnerConstruction();
		OwnerIt->SetActorHiddenInGame(false);
		OwnerIt->SetActorEnableCollision(true);

		// Store to avoid generation on their cells
		DraggedCells.Emplace(MapComponentIt->Cell);
		EActorType ActorType = MapComponentIt->GetActorType();
		if (ActorType == EActorType::Wall)
		{
			DraggedWalls.Emplace(MapComponentIt->Cell);
		}
		else if (ActorType == EActorType::Item)
		{
			DraggedItems.Emplace(MapComponentIt->Cell);
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
		const FIntVector MapScale(GetActorScale3D());
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

		// Locals
		FCells IteratedCells{GridCellsInternal[0]};
		FCells PathBreakers = LDraggedCells = WallsToSpawn.Union(DraggedWalls);

		while (IteratedCells.Num()
		       && !bFoundPath)
		{
			for (const FCell& CellIt : IteratedCells)
			{
				GetSidesCells(PathBreakers, CellIt, EPathType::Explosion, MAX_int32, true);
			}

			IteratedCells = PathBreakers.Difference(LDraggedCells);
			LDraggedCells = PathBreakers;

			if (LCellsToFind.Num())
			{
				LCellsToFind = LCellsToFind.Difference(IteratedCells);
			}

			bFoundPath = !LCellsToFind.Num();
		}

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
		AActor* SpawnedActor = SpawnActorByType(It.Value, It.Key);

#if WITH_EDITOR
		if (USingletonLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld]
		    && SpawnedActor != nullptr)              // Successfully spawn
		{
			// If PIE world, mark this spawned actor as bIsEditorOnlyActor
			SpawnedActor->bIsEditorOnlyActor = true;
			if (UMapComponent* MapComponent = UMapComponent::GetMapComponent(SpawnedActor))
			{
				MapComponent->bIsEditorOnly = true;
			}
		}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]
	}
}

//  Map components getter.
void AGeneratedMap::GetMapComponents(FMapComponents& OutBitmaskedComponents, int32 ActorsTypesBitmask) const
{
	if (!MapComponentsInternal.Num())
	{
		return;
	}

	for (const TObjectPtr<UMapComponent>& MapComponentIt : MapComponentsInternal)
	{
		if (MapComponentIt
		    && EnumHasAnyFlags(MapComponentIt->GetActorType(), TO_ENUM(EActorType, ActorsTypesBitmask)))
		{
			OutBitmaskedComponents.Emplace(MapComponentIt);
		}
	}
}

// Listen game states to generate level actors
void AGeneratedMap::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!World
	    || !PC)
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
	const float CellSize = USingletonLibrary::GetCellSize();

	// Align the Transform
	const FVector NewLocation = UGeneratedMapDataAsset::Get().IsLockedOnZero()
		                            ? FVector::ZeroVector
		                            : Transform.GetLocation().GridSnap(CellSize);
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
			FoundVector *= CellSize;
			// Locate the cell relative to the Level Map
			FoundVector += NewLocation;
			// Subtract the deviation from the center
			FoundVector -= (NewScale3D / 2) * CellSize;
			// Snap to the cell
			FoundVector = FoundVector.GridSnap(CellSize);
			// Cell was found, add rotated cell to the array
			const FCell FoundCell(FCell(FoundVector).RotateAngleAxis(1.f));
			GridCellsInternal.AddUnique(FoundCell);
		}
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
	auto UpdateLevelType = [WeakLevelMap](const FString& Filename, bool bAsTemplate)
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
