// Copyright 2020 Yevhenii Selivanov.

#include "GeneratedMap.h"
//---
#include "BombActor.h"
#include "Bomber.h"
#include "Cell.h"
#include "MapComponent.h"
#include "MyGameInstance.h"
#include "MyGameModeBase.h"
#include "SingletonLibrary.h"
#include "LevelActorDataAsset.h"
//---
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"

/* ---------------------------------------------------
 *		Level map public functions
 * --------------------------------------------------- */

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this actor to call Tick() every time to update characters locations
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.25F;

#if WITH_EDITOR	 //[Editor]
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;
#endif	//WITH_EDITOR [Editor]

	// Initialize the Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetRelativeScale3D(FVector(7.F, 7.F, 7.F));

	// Find blueprint class of the background
	CollisionComponentInternal = CreateDefaultSubobject<UChildActorComponent>(TEXT("Collision Component"));
	CollisionComponentInternal->SetupAttachment(RootComponent);
	static ConstructorHelpers::FClassFinder<AActor> BP_BackgroundAssetFinder(TEXT("/Game/Bomber/Blueprints/BP_CollisionAsset"));
	if (BP_BackgroundAssetFinder.Succeeded())
	{
		BackgroundBlueprintClass = BP_BackgroundAssetFinder.Class; // Default class of the PlatformComponent
	}
}

// Getting an array of cells by four sides of an input center cell and type of breaks
void AGeneratedMap::GetSidesCells(
	FCells& OutCells,
	const FCell& Cell,
	const EPathType Pathfinder,
	const int32& SideLength,
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
			for (const auto& MapComponentIt : BombsMapComponents)
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
				if (bIsY) Distance *= MaxWight;
				const int32 FoundIndex = C0 + Distance;
				if (PositionC0 != (bIsY ? FoundIndex % MaxWight : FoundIndex / MaxWight) // PositionC0 != PositionX
				    || !GridCellsInternal.IsValidIndex(FoundIndex))                             // is not in range
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
AActor* AGeneratedMap::SpawnActorByType(EActorType Type, const FCell& Cell) const
{
	UWorld* World = GetWorld();
	if (!World
	    || ContainsMapComponents(Cell, TO_FLAG(~EAT::Player))	// the free cell was not found
	    || Type == EAT::None)									// nothing to spawn
	{
		return nullptr;
	}

	return World->SpawnActor<AActor>(USingletonLibrary::GetActorClassByType(Type), FTransform(Cell.Location));
}

// The function that places the actor on the Level Map, attaches it and writes this actor to the GridArray_
void AGeneratedMap::AddToGrid(const FCell& Cell, UMapComponent* AddedComponent)
{
	AActor* const ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	const ULevelActorDataAsset* DataAsset = AddedComponent->GetActorDataAsset();
	if (!IS_VALID(ComponentOwner) // the component's owner is not valid or is transient
	    || !DataAsset)            // actor's data asset is now valid
	{
		return;
	}

	if (!MapComponentsInternal.Contains(AddedComponent)) // is not contains in the array
	{
		MapComponentsInternal.Emplace(AddedComponent);
		if (DataAsset->GetActorType() == EAT::Player) // Is a player
		{
			PlayerCharactersNum++;
		}
	}

	ComponentOwner->GetRootComponent()->SetAbsolute(false, false, true);

	// begin: find transform
	FRotator ActorRotation{GetActorRotation()};
	ActorRotation.Yaw += FMath::RandRange(int32(1), int32(4)) * 90.f;
	const FVector ActorLocation{Cell.Location.X, Cell.Location.Y, Cell.Location.Z + 100.f};
	const FVector Scale{1.f, 1.f, 1.f};
	// end

	// Locate actor on cell
	ComponentOwner->SetActorTransform(FTransform(ActorRotation, ActorLocation, Scale));

	// Attach to the Level Map actor
	ComponentOwner->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	USingletonLibrary::PrintToLog(ComponentOwner, "AddToGrid \t ADDED:", Cell.Location.ToString());
}

// The intersection of (OutCells ∩ ActorsTypesBitmask).
void AGeneratedMap::IntersectCellsByTypes(
	FCells& OutCells,
	const int32& ActorsTypesBitmask,
	bool bIntersectAllIfEmpty,
	const UMapComponent* ExceptedComponent) const
{
	if (bIntersectAllIfEmpty == false // should not intersect with all existed cells
	    && OutCells.Num() == 0)       // but the specified array is empty
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
	for (const auto& MapCompIt : BitmaskedComponents)
	{
		if (MapCompIt && MapCompIt != ExceptedComponent)
		{
			BitmaskedCells.Emplace(MapCompIt->Cell);
		}
	}


	OutCells = OutCells.Num() > 0 ? OutCells.Intersect(BitmaskedCells) : BitmaskedCells;
}

// Destroy all actors from the set of cells
void AGeneratedMap::DestroyActorsFromMap(const FCells& Cells, bool bIsNotValidOnly)
{
	if (MapComponentsInternal.Num() == 0 //
	    || !Cells.Num() && !bIsNotValidOnly)
	{
		return;
	}

	for (int32 i = MapComponentsInternal.Num() - 1; i >= 0; --i)
	{
		if (!MapComponentsInternal.IsValidIndex(i)) // the element already was removed
		{
			continue;
		}

		UMapComponent* const MapComponentIt = MapComponentsInternal[i];

		AActor* const ComponentOwner = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		const bool IsValidOwner = IsValid(ComponentOwner);

		// if bIsNotValidOnly param: should destroy only not valid or editor non-PIE owners, otherwise continue
		if (bIsNotValidOnly && IsValidOwner && !ComponentOwner->IsEditorOnly())
		{
			continue;
		}

		if (MapComponentIt && Cells.Contains(MapComponentIt->Cell) // the cell is contained on the grid
		    || bIsNotValidOnly)                                         //  if true, not-valid component should be deleted anyway
		{
			USingletonLibrary::PrintToLog(ComponentOwner, "DestroyActorsFromMap");

			// Remove from the array
			// First removing, because after the box destroying the item can be spawned and starts searching for an empty cell
			RemoveMapComponent(MapComponentIt);

			// Destroy the iterated owner
			if (IsValidOwner)
			{
				ComponentOwner->SetFlags(RF_Transient); // destroy only once
				ComponentOwner->Destroy();
			}
		}
	} // Map components iteration
}

// Removes the specified map component from the MapComponents_ array without an owner destroying
void AGeneratedMap::RemoveMapComponent(UMapComponent* MapComponent)
{
	// Remove from the array
	MapComponentsInternal.Remove(MapComponent);

	if (MapComponent == nullptr) // the Map Component is null
	{
		return;
	}

	// Decrement the players number
	const ULevelActorDataAsset* DataAsset = MapComponent->GetActorDataAsset();
	if (DataAsset
		&& DataAsset->GetActorType() == EAT::Player) // Is a player
	{
		PlayerCharactersNum--;
	}
}

void AGeneratedMap::SetNearestCell(UMapComponent* MapComponent) const
{
	AActor* const ComponentOwner = MapComponent ? MapComponent->GetOwner() : nullptr;
	if (!ensureMsgf(IS_VALID(ComponentOwner), TEXT("FCell:: The specified actor is not valid")))
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

	const int32 InitialCellsNum = CellsToIterate.Num();                                     // The number of initial cells
	FCells NonEmptyCells;                                                                   // all cells of each level actor
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
	for (const auto& CellIt : CellsToIterate)
	{
		Counter++;
		USingletonLibrary::PrintToLog(ComponentOwner, "FCell(MapComponent)", FString::FromInt(Counter) + ":" + CellIt.Location.ToString());

		if (NonEmptyCells.Contains(CellIt)) // the cell is not free from other level actors
		{
			continue;
		}

		// if the cell was found among initial cells without searching a nearest
		if (Counter < InitialCellsNum       // is the initial cell
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

/* ---------------------------------------------------
 *		Level map protected functions
 * --------------------------------------------------- */

// Called every time on this actor to update characters locations on the Level Map.
void AGeneratedMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharactersNum() == 0) // are no players left
	{
		SetActorTickEnabled(false);
		return;
	}

	FMapComponents PlayersMapComponents;
	GetMapComponents(PlayersMapComponents, TO_FLAG(EAT::Player));
	for (const auto& MapCompIt : PlayersMapComponents)
	{
		if (MapCompIt) SetNearestCell(MapCompIt);
	}

	// AI moving
	USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

	// Random item spawning
	// @todo GameMode is null for client, redesign this logic!
	// AMyGameModeBase* MyGameModeBase = USingletonLibrary::GetMyGameMode(this);
	// if (ensureMsgf(MyGameModeBase, TEXT("AGeneratedMap::Tick: MyGameModeBase is null")))
	// {
	// 	const float Timer = MyGameModeBase->Timer;
	// 	const int32 IntTimer = static_cast<int32>(Timer);
	// 	if (IntTimer <= 60                           // after the minute
	// 	    && FMath::IsNearlyEqual(Timer, IntTimer) // is whole number
	// 	    && IntTimer % 10 == 0)                   // each 10 seconds
	// 	{
	// 		const FCell RandCell = *GridCells_[FMath::RandRange(int32(0), GridCells_.Num() - 1)];
	// 		SpawnActorByType(AT::Item, RandCell); // can be not spawned in non empty cell
	// 	}
	// }
}

// Called when an instance of this class is placed (in editor) or spawned
void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this) == true) // the level map is transient
	{
		return;
	}
	USingletonLibrary::PrintToLog(this, "----- OnConstruction -----");

#if WITH_EDITOR	 // [Editor]
	USingletonLibrary::SetLevelMap(this);
#endif	// WITH_EDITOR [Editor]

	// Create the background blueprint child actor
	if (BackgroundBlueprintClass                                    // There is some background class
	    && CollisionComponentInternal                             // Is accessible
	    && !IsValid(CollisionComponentInternal->GetChildActor())) // Is not created yet
	{
		CollisionComponentInternal->SetChildActorClass(BackgroundBlueprintClass);
		CollisionComponentInternal->CreateChildActor();
	}

	// Align the Transform
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	SetActorLocation(GetActorLocation().GridSnap(USingletonLibrary::GetCellSize()));
	FIntVector MapScale(GetActorScale3D());
	if (MapScale.X % 2 != 1) // Length must be unpaired
	{
		MapScale.X += 1;
	}
	if (MapScale.Y % 2 != 1) // Weight must be unpaired
	{
		MapScale.Y += 1;
	}
	MapScale.Z = 1; //Height must be 1
	SetActorScale3D(FVector(MapScale));

	GridCellsInternal.Empty();
	GridCellsInternal.Reserve(MapScale.X * MapScale.Y);

	// Loopy cell-filling of the grid array
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			FVector FoundVector(X, Y, 0.f);
			// Calculate a length of iteration cell
			FoundVector *= USingletonLibrary::GetCellSize();
			// Locate the cell relative to the Level Map
			FoundVector += GetActorLocation();
			// Subtract the deviation from the center
			FoundVector -= GetActorScale3D() / 2 * USingletonLibrary::GetCellSize();
			// Snap to the cell
			FoundVector = FoundVector.GridSnap(USingletonLibrary::GetCellSize());
			// Cell was found, add rotated cell to the array
			const FCell FoundCell(FCell(FoundVector).RotateAngleAxis(1.0F));
			GridCellsInternal.AddUnique(FoundCell);
		}
	}

	// Actors generation
	GenerateLevelActors();
}

// This is called only in the gameplay before calling begin play to generate level actors
void AGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (IS_TRANSIENT(this) == true) // the level map is transient
	{
		return;
	}

	// Update the gameplay LevelMap reference in the singleton library
	USingletonLibrary::SetLevelMap(this);

	// Cells regeneration if the level map's size was changed
	UMyGameInstance* const MyGameInstance = USingletonLibrary::GetMyGameInstance(this);
	if (MyGameInstance)
	{
		const FVector MapScale = MyGameInstance->LevelMapScale;
		if (MapScale.IsZero() == false // is not zero
		    && MapScale != GetActorScale3D())
		{
			SetActorScale3D(MapScale);
		}

		RerunConstructionScripts();
	}
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	check(GridCellsInternal.Num() > 0 && "Is no cells for the actors generation");
	USingletonLibrary::PrintToLog(this, "----- GenerateLevelActors ------", "---- START -----");

	// Destroy all editor-only non-PIE actors
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::All));
	DestroyActorsFromMap(NonEmptyCells, true);
	NonEmptyCells.Empty();

	// Calls before generation preview actors to updating of all dragged to the Level Map actors
	// After destroying only editor actors and before their generation
	for (const auto& MapComponentIt : MapComponentsInternal)
	{
		MapComponentIt->RerunOwnerConstruction();
		NonEmptyCells.Emplace(MapComponentIt->Cell);
	}
	USingletonLibrary::PrintToLog(this, "_____ [Editor]BroadcastActorsUpdating _____", "_____ END _____");

	// Set number of existed player characters
	FMapComponents PlayersMapComponents;
	GetMapComponents(PlayersMapComponents, TO_FLAG(EAT::Player));
	PlayerCharactersNum = PlayersMapComponents.Num();

	/* Steps:
	 *
	 * Part 0: Actors random filling to the ArrayToGenerate.
	 * 0.1) Finding all symmetrical cells for each iterated cell;
	 *
	 * Part 1: Checking if there is a path to the each bone. If not, go to the 0 step.
	 *
	 * Part 2: Spawning these actors
	 */

	// Locals
	const FIntVector MapScale(GetActorScale3D());
	const FIntVector MapHalfScale(MapScale / 2);
	FCells Bones;
	TMap<FCell, EActorType> ActorsToSpawn;

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
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "PLAYER will be spawned");
				ActorTypeToSpawn = EAT::Player;
			}

			// Wall condition
			if (ActorTypeToSpawn == EAT::None // all previous conditions are false
			    && !IsSafeZone && FMath::RandRange(int32(0), int32(99)) < WallsChance_)
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "WALL will be spawned");
				ActorTypeToSpawn = EAT::Wall;
			}

			// Box condition
			if (ActorTypeToSpawn == EAT::None                                    // all previous conditions are false
			    && !IsSafeZone && FMath::RandRange(int32(0), int32(99)) < BoxesChance_) // Chance of boxes
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "BOX will be spawned");
				ActorTypeToSpawn = EAT::Box;
			}

			if (ActorTypeToSpawn != EAT::Wall) //
			{
				Bones.Emplace(CellIt);
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
						default: break;
					}

					CellIt = GridCellsInternal[MapScale.X * Yi + Xi];
				}

				if (!NonEmptyCells.Contains(CellIt)) // the cell is not free
				{
					ActorsToSpawn.Emplace(CellIt, ActorTypeToSpawn);
				}
				else if (ActorTypeToSpawn == EAT::Wall)
				{
					Bones.Emplace(CellIt); // the wall was not spawned, then this cell is the bone
				}
			} // Symmetry iterations
		}     // X iterations
	}         // Y iterations

	// --- Part 1 : Checking if there is a path to the bottom and side edges.If not, go to the 0 step._ ---

	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::Wall));
	Bones = Bones.Difference(NonEmptyCells);

	// Finding all cells that not in Bones (NonEmptyCells = GridCells / Bones)
	for (const FCell& CellIt : GridCellsInternal)
	{
		if (!Bones.Contains(CellIt))
		{
			NonEmptyCells.Emplace(CellIt);
		}
	}

	// Locals
	FCells IteratedCells{GridCellsInternal[0]}, FinishedCells = NonEmptyCells;

// #if WITH_EDITOR // show generated bones (green and red)
// 	if (bShouldShowRenders)
// 	{
// 		bool bOutBool = false;
// 		TArray<UTextRenderComponent*> OutArray{};
// 		USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, Bones, FLinearColor::Green, bOutBool, OutArray, 263, 148);
// 		USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, NonEmptyCells, FLinearColor::Red, bOutBool, OutArray, 260);
// 	}
// #endif	// WITH_EDITOR

	while (IteratedCells.Num() && Bones.Num())
	{
		for (const FCell& CellIt : IteratedCells)
		{
			GetSidesCells(FinishedCells, CellIt, EPathType::Explosion, 100, true);
		}
		IteratedCells = FinishedCells.Difference(NonEmptyCells);
		Bones = Bones.Difference(FinishedCells);
		NonEmptyCells = FinishedCells;
	}

	if (Bones.Num())
	{
		GenerateLevelActors();
		return;
	}

	// --- Part 2: Spawning ---

	for (const auto& It : ActorsToSpawn)
	{
		AActor* SpawnedActor = SpawnActorByType(It.Value, It.Key);

#if WITH_EDITOR
		if (USingletonLibrary::IsEditorNotPieWorld() // [IsEditorNotPieWorld]
		    && SpawnedActor != nullptr)              // Successfully spawn
		{
			// If PIE world, mark this spawned actor as bIsEditorOnlyActor
			SpawnedActor->bIsEditorOnlyActor = true;
			UMapComponent::GetMapComponent(SpawnedActor)->bIsEditorOnly = true;
		}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		USingletonLibrary::ClearOwnerTextRenders(this);
		// Show cells coordinates of the Grid array
		if (bShouldShowRenders)
		{
			FCells CellsToRender;
			IntersectCellsByTypes(CellsToRender, RenderActorsTypes, true);
			USingletonLibrary::AddDebugTextRenders(this, CellsToRender.Array());
		}
	}
#endif	// WITH_EDITOR [IsEditorNotPieWorld]

	USingletonLibrary::PrintToLog(this, "_____ GenerateLevelActors _____", "_____ END _____");
}

//  Map components getter.
void AGeneratedMap::GetMapComponents(FMapComponents& OutBitmaskedComponents, const int32& ActorsTypesBitmask) const
{
	if (MapComponentsInternal.Num() > 0)
	{
		OutBitmaskedComponents.Append(
			MapComponentsInternal.FilterByPredicate([ActorsTypesBitmask](const UMapComponent* Ptr)
			{
				const ULevelActorDataAsset* DataAsset = Ptr ? Ptr->GetActorDataAsset() : nullptr;
				const EActorType ActorType = DataAsset ? DataAsset->GetActorType() : EAT::None;
				return EnumHasAnyFlags(ActorType, TO_ENUM(EActorType, ActorsTypesBitmask));
			}));
	}
}

/* ---------------------------------------------------
 *					Editor development
 * --------------------------------------------------- */

#if WITH_EDITOR	 // [Editor] Destroyed
// Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending
void AGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this))
	{
		FCells NonEmptyCells;
		IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EAT::All));
		DestroyActorsFromMap(NonEmptyCells);
	}
	Super::Destroyed();
}
#endif	// [Editor] Destroyed
