// Copyright 2019 Yevhenii Selivanov.

#include "GeneratedMap.h"

#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"

#include "BombActor.h"
#include "Bomber.h"
#include "Cell.h"
#include "MapComponent.h"
#include "MyGameInstance.h"
#include "MyGameModeBase.h"
#include "SingletonLibrary.h"

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

#if WITH_EDITOR  //[Editor]
	// Should not call OnConstruction on drag events
	bRunConstructionScriptOnDrag = false;
#endif  //WITH_EDITOR [Editor]

	// Initialize the Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->RelativeScale3D = FVector(7.F, 7.F, 7.F);

	// Find blueprint class of the background
	BackgroundBlueprintComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("BackgroundBlueprintComponent"));
	BackgroundBlueprintComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FClassFinder<AActor> BackgroundClassFinder(TEXT("/Game/Bomber/Assets/BackgroundBlueprintAsset"));
	if (BackgroundClassFinder.Succeeded())
	{
		BackgroundBlueprintClass = BackgroundClassFinder.Class;  // Default class of the PlatformComponent
	}
}

// Getting an array of cells by four sides of an input center cell and type of breaks
void AGeneratedMap::GetSidesCells(
	FCells& OutCells,
	const FCell& Cell,
	const EPathType& Pathfinder,
	const int32& SideLength) const
{
	OutCells.Empty();

	// ----- Locals -----
	FCells Walls;
	IntersectCellsByTypes(Walls, TO_FLAG(EActorType::Wall));

	// the index of the specified cell
	const int32 C0 = GridCells_.IndexOfByPredicate([Cell](const FSharedCell& SharedCell) { return *SharedCell == Cell; });
	if (C0 == INDEX_NONE)  // if index was found and cell is contained in the array
	{
		return;
	}

	// ----- A path without obstacles -----
	FCells Obstacles;
	const bool bWithoutObstacles = Pathfinder != EPathType::Explosion;
	if (bWithoutObstacles)  // if is the request to find the path without Bombs/Boxes
	{
		IntersectCellsByTypes(Obstacles, TO_FLAG(EActorType::Bomb | EActorType::Box));
	}

	// ----- Secure: a path without players -----
	FCells PlayersCells;
	const bool bWithoutPlayers = Pathfinder == EPathType::Secure;
	if (bWithoutPlayers)  // if is the request to find the path without players cells.
	{
		IntersectCellsByTypes(PlayersCells, TO_FLAG(EActorType::Player));
	}

	// ----- A path without explosions -----
	FCells DangerousCells;
	const bool bWithoutExplosions = Pathfinder == EPathType::Safe || Pathfinder == EPathType::Secure;
	if (bWithoutExplosions)  // if is the request to find the path without explosions.
	{
		FMapComponents BombsMapComponents;
		GetMapComponents(BombsMapComponents, TO_FLAG(EActorType::Bomb));
		if (BombsMapComponents.Num() > 0)
		{
			for (const auto& MapComponentIt : BombsMapComponents)
			{
				const auto BombOwner = MapComponentIt ? Cast<ABombActor>(MapComponentIt->GetOwner()) : nullptr;
				if (IS_VALID(BombOwner))  // is valid and is not transient
				{
					DangerousCells = DangerousCells.Union(BombOwner->ExplosionCells_);
				}
			}
		}
	}

	// ----- The specified cell adding -----
	if (bWithoutExplosions == false			// can be danger
		|| !DangerousCells.Contains(Cell))  // is not dangerous cell
	{
		OutCells.Emplace(Cell);
	}

	// ----- Cells finding -----
	const int32 MaxWight = GetActorScale3D().X;
	for (int32 bIsY = 0; bIsY <= 1; ++bIsY)  // 0(X-raw direction) and 1(Y-column direction)
	{
		const int32 PositionC0 = bIsY ? /*Y-column*/ C0 % MaxWight : C0 / MaxWight /*raw*/;
		for (int32 SideMultiplier = -1; SideMultiplier <= 1; SideMultiplier += 2)  // -1(Left|Down) and 1(Right|Up)
		{
			for (int32 i = 1; i <= SideLength; ++i)
			{
				int32 Distance = i * SideMultiplier;
				if (bIsY) Distance *= MaxWight;
				const int32 FoundIndex = C0 + Distance;
				if (PositionC0 != (bIsY ? FoundIndex % MaxWight : FoundIndex / MaxWight)  // PositionC0 != PositionX
					|| !GridCells_.IsValidIndex(FoundIndex))							  // is not in range
				{
					break;  // to the next side
				}

				const FCell FoundCell = *GridCells_[FoundIndex];

				if (Walls.Contains(FoundCell)									  // cell contains a wall
					|| bWithoutObstacles && Obstacles.Contains(FoundCell)		  // cell contains an obstacle (Bombs/Boxes)
					|| bWithoutPlayers && PlayersCells.Contains(FoundCell)		  // cell contains a player
					|| bWithoutExplosions && DangerousCells.Contains(FoundCell))  // cell contains an explosion
				{
					break;  // to the next side
				}

				OutCells.Emplace(FoundCell);
			}  // Cells iterating

		}  // Each side iterating: -1(Left|Down) and 1(Right|Up)

	}  // Each direction iterating: 0(X-raw) and 1(Y-column)
}

// Spawns level actor on the Level Map by the specified type
AActor* AGeneratedMap::SpawnActorByType(const EActorType& Type, const FCell& Cell)
{
	UWorld* World = GetWorld();
	if (!World || ContainsMapComponents(Cell, TO_FLAG(~EActorType::Player))  // the free cell was not found
		|| Type == EActorType::None)										 // nothing to spawn
	{
		return nullptr;
	}

	return World->SpawnActor<AActor>(USingletonLibrary::GetClassByActorType(Type), FTransform(Cell.Location));
}

// The function that places the actor on the Level Map, attaches it and writes this actor to the GridArray_
void AGeneratedMap::AddToGrid(const FCell& Cell, UMapComponent* AddedComponent)
{
	AActor* const ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!IS_VALID(ComponentOwner))  // the component's owner is not valid or is transient
	{
		return;
	}

	if (!MapComponents_.Contains(AddedComponent))  // is not contains in the array
	{
		MapComponents_.Emplace(AddedComponent);

		if (AddedComponent->ActorType == EActorType::Player)  // Is a player
		{
			PlayerCharactersNum++;
		}
	}

	ComponentOwner->GetRootComponent()->SetAbsolute(false, false, true);

	// Locate actor on cell
	FRotator ActorRotation{GetActorRotation()};
	ActorRotation.Yaw += FMath::RandRange(int32(1), int32(4)) * 90.f;
	const FVector ActorLocation{Cell.Location.X, Cell.Location.Y, Cell.Location.Z + 100.f};
	const FVector Scale{1.f, 1.f, 1.f};
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
	if (bIntersectAllIfEmpty == false  // should not intersect with all existed cells
		&& OutCells.Num() == 0)		   // but the specified array is empty
	{
		return;
	}

	FMapComponents BitmaskedComponents;
	GetMapComponents(BitmaskedComponents, ActorsTypesBitmask);
	if (BitmaskedComponents.Num() == 0)
	{
		OutCells.Empty();  // nothing found, returns empty OutCells array
		return;
	}

	FCells BitmaskedCells;
	for (const auto& MapCompIt : BitmaskedComponents)
	{
		if (MapCompIt && MapCompIt != ExceptedComponent) BitmaskedCells.Emplace(MapCompIt->GetCell());
	}

	OutCells = OutCells.Num() > 0 ? OutCells.Intersect(BitmaskedCells) : BitmaskedCells;
}

// Destroy all actors from the set of cells
void AGeneratedMap::DestroyActorsFromMap(const FCells& Cells, bool bIsNotValidOnly)
{
	if (MapComponents_.Num() == 0  //
		|| Cells.Num() == 0)
	{
		return;
	}

	for (int32 i = MapComponents_.Num() - 1; i >= 0; --i)
	{
		if (!MapComponents_.IsValidIndex(i))  // the element already was removed
		{
			continue;
		}

		UMapComponent* const MapComponentIt = MapComponents_[i];

		AActor* const ComponentOwner = MapComponentIt ? MapComponentIt->GetOwner() : nullptr;
		const bool IsValidOwner = IsValid(ComponentOwner);

		// if bIsNotValidOnly param: should destroy only not valid or editor non-PIE owners, otherwise continue
		if (bIsNotValidOnly && IsValidOwner && !ComponentOwner->IsEditorOnly())
		{
			continue;
		}

		if (MapComponentIt && Cells.Contains(MapComponentIt->GetCell())  // the cell is contained on the grid
			|| bIsNotValidOnly)											 //  if true, not-valid component should be deleted anyway
		{
			USingletonLibrary::PrintToLog(ComponentOwner, "DestroyActorsFromMap");

			// Remove from the array
			// First removing, because after the box destroying the item can be spawned and starts searching for an empty cell
			RemoveMapComponent(MapComponentIt);

			// Destroy the iterated owner
			if (IsValidOwner)
			{
				ComponentOwner->SetFlags(RF_Transient);  // destroy only once
				ComponentOwner->Destroy();
			}
		}
	}  // Map components iteration
}

// Removes the specified map component from the MapComponents_ array without an owner destroying
void AGeneratedMap::RemoveMapComponent(UMapComponent* MapComponent)
{
	// Remove from the array
	MapComponents_.Remove(MapComponent);

	if (MapComponent == nullptr)  // the Map Component is null
	{
		return;
	}

	// Remove the weak reference
	MapComponent->Cell.Reset();

	// Decrement the players number
	if (MapComponent->ActorType == EActorType::Player)  // Is a player
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
	FSharedCell FoundCell;
	const FCell OwnerCell(ComponentOwner->GetActorLocation());  // The owner location
	// Check if the owner already standing on:
	FCells InitialCells({OwnerCell,																		   // 0: exactly the current his cell
		FCell(OwnerCell.RotateAngleAxis(-1.F).Location.GridSnap(FCell::CellSize)).RotateAngleAxis(1.F)});  // 1: within the radius of one cell

	TSet<FSharedCell> CellsToIterate(GridCells_.FilterByPredicate([InitialCells](FSharedCell Cell) {
		return InitialCells.Contains(*Cell);
	}));

	const int32 InitialCellsNum = CellsToIterate.Num();										 // The number of initial cells
	FCells NonEmptyCells;																	 // all cells of each level actor
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(~EActorType::Player), true, MapComponent);  //EActorType::Bomb | EActorType::Item | EActorType::Wall | EActorType::Box

	// Pre gameplay locals to find a nearest cell
	const bool bHasNotBegunPlay = !HasActorBegunPlay();  // the game was not started
	float LastFoundEditorLen = MAX_FLT;
	if (bHasNotBegunPlay)
	{
		CellsToIterate.Append(GridCells_);  // union of two sets(initials+all) for finding a nearest cell
	}

	// ----- Part 1:  Cells iteration

	int32 Counter = -1;
	for (const auto& CellIt : CellsToIterate)
	{
		Counter++;
		USingletonLibrary::PrintToLog(ComponentOwner, "FCell(MapComponent)", FString::FromInt(Counter) + ":" + CellIt->Location.ToString());

		if (NonEmptyCells.Contains(*CellIt))  // the cell is not free from other level actors
		{
			continue;
		}

		// if the cell was found among initial cells without searching a nearest
		if (Counter < InitialCellsNum		 // is the initial cell
			&& GridCells_.Contains(CellIt))  // is contained on the grid
		{
			FoundCell = CellIt;
			break;
		}

		//	Finding the nearest cell before starts the game
		if (bHasNotBegunPlay				// the game was not started
			&& Counter >= InitialCellsNum)  // if iterated cell is not initial
		{
			const float EditorLenIt = USingletonLibrary::CalculateCellsLength(OwnerCell, *CellIt);
			if (EditorLenIt < LastFoundEditorLen)  // Distance closer
			{
				LastFoundEditorLen = EditorLenIt;
				FoundCell = CellIt;
			}
		}

	}  //[Cells Iteration]

	// Checks the cell is contained in the grid and free from other level actors.
	if (FoundCell.IsValid())
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
	if (GetCharactersNum() == 0)  // are no players left
	{
		SetActorTickEnabled(false);
		return;
	}

	FMapComponents PlayersMapComponents;
	GetMapComponents(PlayersMapComponents, TO_FLAG(EActorType::Player));
	for (const auto& MapCompIt : PlayersMapComponents)
	{
		if (MapCompIt) SetNearestCell(MapCompIt);
	}

	// AI moving
	USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();

	// Random item spawning
	const float Timer = USingletonLibrary::GetMyGameMode(this)->Timer;
	const int32 IntTimer = static_cast<int32>(Timer);
	if (IntTimer <= 60							  // after the minute
		&& FMath::IsNearlyEqual(Timer, IntTimer)  // is whole number
		&& IntTimer % 10 == 0)					  // each 10 seconds
	{
		const FCell RandCell = *GridCells_[FMath::RandRange(int32(0), GridCells_.Num() - 1)];
		SpawnActorByType(EActorType::Item, RandCell);  // can be not spawned in non empty cell
	}
}

// Called when an instance of this class is placed (in editor) or spawned
void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IS_TRANSIENT(this) == true)  // the level map is transient
	{
		return;
	}
	USingletonLibrary::PrintToLog(this, "----- OnConstruction -----");

#if WITH_EDITOR  // [Editor]
	USingletonLibrary::SetLevelMap(this);
#endif  // WITH_EDITOR [Editor]

	// Create the background blueprint child actor
	if (BackgroundBlueprintClass									 // There is some background class
		&& BackgroundBlueprintComponent								 // Is accessible
		&& !IsValid(BackgroundBlueprintComponent->GetChildActor()))  // Is not created yet
	{
		BackgroundBlueprintComponent->SetChildActorClass(BackgroundBlueprintClass);
		BackgroundBlueprintComponent->CreateChildActor();
	}

	// Align the Transform
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	SetActorLocation(GetActorLocation().GridSnap(USingletonLibrary::GetCellSize()));
	FIntVector MapScale(GetActorScale3D());
	if (MapScale.X % 2 != 1)  // Length must be unpaired
	{
		MapScale.X += 1;
	}
	if (MapScale.Y % 2 != 1)  // Weight must be unpaired
	{
		MapScale.Y += 1;
	}
	MapScale.Z = 1;  //Height must be 1
	SetActorScale3D(FVector(MapScale));

	// Clear the old grid array
	for (auto& SharedCell : GridCells_)
	{
		SharedCell.Reset();
	}
	GridCells_.Empty();
	GridCells_.Reserve(MapScale.X * MapScale.Y);

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
			GridCells_.AddUnique(MakeShared<FCell>(FoundCell));
		}
	}

#if WITH_EDITOR  // [IsEditorNotPieWorld] \
	// Show cells coordinates of the Grid array
	USingletonLibrary::ClearOwnerTextRenders(this);
	if (bShouldShowRenders)
	{
		USingletonLibrary::AddDebugTextRenders(this, GridCells_);
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]

	// Actors generation
	GenerateLevelActors();
}

// This is called only in the gameplay before calling begin play to generate level actors
void AGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (IS_TRANSIENT(this) == true)  // the level map is transient
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
		if (MapScale.IsZero() == false  // is not zero
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
	check(GridCells_.Num() > 0 && "Is no cells for the actors generation");
	USingletonLibrary::PrintToLog(this, "----- GenerateLevelActors ------", "---- START -----");
	USingletonLibrary::ClearOwnerTextRenders(this);

	// Destroy all editor-only non-PIE actors
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EActorType::All));
	DestroyActorsFromMap(NonEmptyCells, true);
	NonEmptyCells.Empty();

	// Calls before generation preview actors to updating of all dragged to the Level Map actors
	// After destroying only editor actors and before their generation
	for (const auto& MapComponentIt : MapComponents_)
	{
		MapComponentIt->RerunOwnerConstruction();
		NonEmptyCells.Emplace(MapComponentIt->GetCell());
	}
	USingletonLibrary::PrintToLog(this, "_____ [Editor]BroadcastActorsUpdating _____", "_____ END _____");

	// Set number of existed player characters
	FMapComponents PlayersMapComponents;
	GetMapComponents(PlayersMapComponents, TO_FLAG(EActorType::Player));
	PlayerCharactersNum = PlayersMapComponents.Num();

	/* Steps:
	 * 
	 * Part 0: Actors random filling to the ArrayToGenerate.
	 *
	 * Part 1: ArrayToGenerate iteration:
	 * 1.1) Finding all symmetrical cells for each iterated cell;
	 * 1.2) Spawning these actors
	 *
	 * Part 2: Checking if there is a path to the bottom and side edges. If not, go to the 0 step.
	 */

	// Locals
	const FIntVector MapScale(GetActorScale3D());
	const FIntVector MapHalfScale(MapScale / 2);
	FCells EndCellsX, EndCellsY;

	for (int32 Y = 0; Y <= MapHalfScale.Y; ++Y)  // Strings
	{
		for (int32 X = 0; X <= MapHalfScale.X; ++X)  // Columns
		{
			if (X == 0 && Y == 1 || X == 1 && Y == 0)  // is the safe zone
			{
				continue;  // skip
			}

			// Filling the bottom and side cells
			FCell CellIt = *GridCells_[MapScale.X * Y + X];
			if (X != Y)
			{
				if (X == MapHalfScale.X)
				{
					EndCellsX.Emplace(CellIt);
				}
				else if (Y == MapHalfScale.Y)
				{
					EndCellsY.Emplace(CellIt);
				}
			}

			// --- Part 0: Actors random filling to the ArrayToGenerate._ ---

			// In case all next conditions will be false
			EActorType ActorTypeToSpawn = EActorType::None;

			// Player condition
			if (X == 0 && Y == 0)  // is first corner
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "PLAYER will be spawned");
				ActorTypeToSpawn = EActorType::Player;
			}

			// Wall condition
			if (ActorTypeToSpawn == EActorType::None  // all previous conditions are false
				&& FMath::RandRange(int32(0), int32(99)) < WallsChance_)
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "WALL will be spawned");
				ActorTypeToSpawn = EActorType::Wall;
			}

			// Box condition
			if (ActorTypeToSpawn == EActorType::None					  // all previous conditions are false
				&& FMath::RandRange(int32(0), int32(99)) < BoxesChance_)  // Chance of boxes
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "BOX will be spawned");
				ActorTypeToSpawn = EActorType::Box;
			}

			// Adds to the array
			if (ActorTypeToSpawn == EActorType::None)  // There is no types to spawn
			{
				continue;
			}

			// --- Part 1: ArrayToGenerate iteration ---
			const int32 Xs = MapScale.X - 1 - X, Ys = MapScale.Y - 1 - Y;  // Symmetrized cell position

			// 1.1) Array symmetrization
			bool IsEmptyCell = true;
			FCells CellsForSpawning;
			for (int32 I = 0; I < 4; ++I)  // 4 sides of symmetry
			{
				if (I > 0)  // the 0 index is always current CellIt, otherwise needs to find symmetry
				{
					int32 Xi = X, Yi = Y;  // Keeping the current coordinates
					switch (I)
					{
						case 1:  // (X1 = Xs; Y1 = Y)
							Xi = Xs;
							break;
						case 2:  // (X2 = X; Y2 = Ys)
							Yi = Ys;
							break;
						case 3:  // (X3 = Xs; Y3 = Ys)
							Xi = Xs;
							Yi = Ys;
							break;
						default: break;
					}

					CellIt = *GridCells_[MapScale.X * Yi + Xi];
				}

				if (NonEmptyCells.Contains(CellIt))  // the cell is not free
				{
					IsEmptyCell = false;
					break;
				}

				CellsForSpawning.Emplace(CellIt);
			}

			if (!IsEmptyCell)  // the one of symmetry cells is not free
			{
				continue;  // skip all symmetry CellsForSpawning for this iteration
			}

			// 1.2) Spawning
			for (const auto& CellToSpawn : CellsForSpawning)
			{
				AActor* SpawnedActor = SpawnActorByType(ActorTypeToSpawn, CellToSpawn);

#if WITH_EDITOR
				if (USingletonLibrary::IsEditorNotPieWorld()  // [IsEditorNotPieWorld]
					&& SpawnedActor != nullptr)				  // Successfully spawn
				{
					// If PIE world, mark this spawned actor as bIsEditorOnlyActor
					SpawnedActor->bIsEditorOnlyActor = true;
					UMapComponent::GetMapComponent(SpawnedActor)->bIsEditorOnly = true;
				}
#endif		   // WITH_EDITOR [IsEditorNotPieWorld]
			}  // Spawning iterations

		}  // X iterations
	}	  // Y iterations

	// --- Part 2 : Checking if there is a path to the bottom and side edges.If not, go to the 0 step._ ---

#if WITH_EDITOR
	bool bOutBool = false;
	TArray<UTextRenderComponent*> OutArray{};
	USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, EndCellsX, FLinearColor::Blue, bOutBool, OutArray);
	USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, EndCellsY, FLinearColor::Green, bOutBool, OutArray);
#endif  // WITH_EDITOR

	// Locals
	bool bIsConnectedX = false, bIsConnectedY = false;
	FCells IteratedCells, FinishedCells;
	GetSidesCells(IteratedCells, *GridCells_[0], EPathType::Explosion, 100);

	while (IteratedCells.Num() > 0)
	{
#if WITH_EDITOR
		USingletonLibrary::GetSingleton()->AddDebugTextRenders(this, IteratedCells, FLinearColor::Black, bOutBool, OutArray);
#endif  // WITH_EDITOR

		FCells FoundCells;

		for (const auto& CellIt : IteratedCells)
		{
			FinishedCells.Emplace(CellIt);

			if (!bIsConnectedX)
			{
				bIsConnectedX = EndCellsX.Contains(CellIt);
			}

			if (!bIsConnectedY)
			{
				bIsConnectedY = EndCellsY.Contains(CellIt);
			}

			if (bIsConnectedX && bIsConnectedY)
			{
				USingletonLibrary::PrintToLog(this, "_____ GenerateLevelActors _____", "_____ END _____");
				return;
			}

			GetSidesCells(FoundCells, CellIt, EPathType::Explosion, 100);
		}

		IteratedCells = FoundCells.Difference(FinishedCells);
	}

	// Regenarate if the grid is disconnected
	UGameplayStatics::OpenLevel(this, "BomberLevel");
}

//  Map components getter.
void AGeneratedMap::GetMapComponents(FMapComponents& OutBitmaskedComponents, const int32& ActorsTypesBitmask) const
{
	if (MapComponents_.Num() > 0)
	{
		OutBitmaskedComponents.Append(
			MapComponents_.FilterByPredicate([ActorsTypesBitmask](const UMapComponent* Ptr) {
				return Ptr && Ptr->ActorType & ActorsTypesBitmask;
			}));
	}
}

/* ---------------------------------------------------
 *					Editor development
 * --------------------------------------------------- */

#if WITH_EDITOR  // [Editor] Destroyed

// Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending
void AGeneratedMap::Destroyed()
{
	if (!IS_TRANSIENT(this))
	{
		FCells NonEmptyCells;
		IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EActorType::All));
		DestroyActorsFromMap(NonEmptyCells);
	}
	Super::Destroyed();
}
#endif  // [Editor] Destroyed
