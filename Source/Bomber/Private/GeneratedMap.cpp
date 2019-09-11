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
	RootComponent->RelativeScale3D = FVector(5.f, 5.f, 1.f);

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
	const int32 C0 = GridCells_.IndexOfByKey(Cell);  // the index of the specified cell
	if (C0 == INDEX_NONE)							 // if index was found and cell is contained in the array
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
				if (PositionC0 != (bIsY ? FoundIndex % MaxWight : FoundIndex / MaxWight)  // // PositionC0) != PositionX
					|| !GridCells_.IsValidIndex(FoundIndex))							  // is not in range
				{
					break;  // to the next side
				}

				const FCell FoundCell = GridCells_[FoundIndex];

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
	if (!World || !Cell.bWasFound)  // the free cell was not found
	{
		return nullptr;
	}

	return World->SpawnActor<AActor>(USingletonLibrary::GetClassByActorType(Type), FTransform(Cell.Location));
}

// The function that places the actor on the Level Map, attaches it and writes this actor to the GridArray_
void AGeneratedMap::AddToGrid(const FCell& Cell, UMapComponent* AddedComponent)
{
	AActor* const ComponentOwner = AddedComponent ? AddedComponent->GetOwner() : nullptr;
	if (!IS_VALID(ComponentOwner)  // the component's owner is not valid or is transient
		|| !Cell.bWasFound)		   // the free cell was not found
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
			// First remove, because after the box destroying the item can be spawned and starts searching for an empty cell
			MapComponents_.RemoveAt(i, 1, false);

			// Destroy the iterated owner
			if (IsValidOwner)
			{
				ComponentOwner->SetFlags(RF_Transient);  // destroy only once
				ComponentOwner->Destroy();
			}

			// Decrement the players number
			if (MapComponentIt && MapComponentIt->ActorType == EActorType::Player)  // Is a player
			{
				PlayerCharactersNum--;
			}
		}
	}  // Map components iteration

	MapComponents_.Shrink();
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
		if (MapCompIt) MapCompIt->UpdateCell();
	}

	// AI moving
	USingletonLibrary::GetSingleton()->OnAIUpdatedDelegate.Broadcast();
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
			GridCells_.AddUnique(FoundCell);
		}
	}

#if WITH_EDITOR  // [IsEditorNotPieWorld] \
	// Show cells coordinates of the Grid array
	if (bShouldShowRenders == true)
	{
		USingletonLibrary::ClearOwnerTextRenders(this);
		USingletonLibrary::AddDebugTextRenders(this, FCells(GridCells_));
	}

	// Preview generation
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		GenerateLevelActors();
	}
#endif  // WITH_EDITOR [IsEditorNotPieWorld]
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
			RerunConstructionScripts();
		}
	}

	// Actors generation
	GenerateLevelActors();
}

// Spawns and fills the Grid Array values by level actors
void AGeneratedMap::GenerateLevelActors()
{
	if (GridCells_.Num() == 0)
	{
		check("Is no cells for the actors generation");
		return;
	}
	USingletonLibrary::PrintToLog(this, "----- GenerateLevelActors ------", "---- START -----");

	// Destroy all editor-only non-PIE actors
	DestroyActorsFromMap(FCells(GridCells_), true);

#if WITH_EDITOR  // [Editor]
	// After destroying PIE actors and before their generation,
	// calling to updating of all dragged to the Level Map actors
	USingletonLibrary::BroadcastActorsUpdating();  // [IsEditorNotPieWorld]
	USingletonLibrary::PrintToLog(this, "_____ [Editor]BroadcastActorsUpdating _____", "_____ END _____");
#endif  // [Editor]

	// Set number of existed player characters
	FMapComponents PlayersMapComponents;
	GetMapComponents(PlayersMapComponents, TO_FLAG(EActorType::Player));
	PlayerCharactersNum = PlayersMapComponents.Num();

	// Getting all non empty cells of each actor.
	FCells NonEmptyCells;
	IntersectCellsByTypes(NonEmptyCells, TO_FLAG(EActorType::All));

	// Cells iterating by rows
	const FIntVector MapScale(GetActorScale3D());  // Iterating by sizes (strings and columns)
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			const FCell CellIt = GridCells_[MapScale.X * Y + X];
			USingletonLibrary::PrintToLog(this, "GenerateLevelActors \t Iterated cell:", CellIt.Location.ToString());
			if (NonEmptyCells.Contains(CellIt))
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors \t The actor on the cell has already existed");
				continue;
			}
			// --- Part 0: Selection ---

			// In case all next conditions will be false
			EActorType ActorTypeToSpawn = EActorType::None;

			// Wall condition
			if (X % 2 == 1 && Y % 2 == 1)
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "WALL will be spawned");
				ActorTypeToSpawn = EActorType::Wall;
			}

			// Player condition
			const bool bIsCornerX = (X == 0 || X == MapScale.X - 1);
			const bool bIsCornerY = (Y == 0 || Y == MapScale.Y - 1);
			if (bIsCornerX && bIsCornerY)  // is one of the corners
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "PLAYER will be spawned");
				ActorTypeToSpawn = EActorType::Player;
			}

			// Box condition
			if (ActorTypeToSpawn == EActorType::None					  // all previous conditions are false
				&& FMath::RandRange(int32(0), int32(99)) < BoxesChance_   // Chance of boxes
				&& (!bIsCornerX && X != 1 && X != MapScale.X - 2		  // X corner zone
					   || !bIsCornerY && Y != 1 && Y != MapScale.Y - 2))  // Y corner zone
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors", "BOX will be spawned");
				ActorTypeToSpawn = EActorType::Box;
			}

			// --- Part 1: Spawning ---

			if (ActorTypeToSpawn != EActorType::None)  // There is type to spawn
			{
				AActor* SpawnedActor = SpawnActorByType(ActorTypeToSpawn, CellIt);
#if WITH_EDITOR
				if (USingletonLibrary::IsEditorNotPieWorld()  // [IsEditorNotPieWorld]
					&& SpawnedActor != nullptr)				  // Successfully spawn
				{
					// If PIE world, mark this spawned actor as bIsEditorOnlyActor
					SpawnedActor->bIsEditorOnlyActor = true;
					UMapComponent::GetMapComponent(SpawnedActor)->bIsEditorOnly = true;
					USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.RemoveAll(SpawnedActor);
				}
#endif		   // WITH_EDITOR [IsEditorNotPieWorld]
			}  // End of the spawn part
		}	  // X iterations
	}		   // Y iterations

	USingletonLibrary::PrintToLog(this, "_____ GenerateLevelActors _____", "_____ END _____");
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
		DestroyActorsFromMap(FCells(GridCells_));
	}
	Super::Destroyed();
}
#endif  // [Editor] Destroyed