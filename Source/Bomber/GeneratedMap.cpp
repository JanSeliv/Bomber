// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

#if WITH_EDITOR		 // [Editor]
#include "Editor.h"  // FEditorDelegates::EndPIE
#endif				 //WITH_EDITOR [Editor]

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Should not call OnConstruction on drag events
#if WITH_EDITOR  // [Editor] bRunConstructionScriptOnDrag
	bRunConstructionScriptOnDrag = false;

	// Updating the level map reference in the singleton library
	// Register callback to updating the level map reference and listening actors on PIE events
	FEditorDelegates::EndPIE.AddLambda([this](bool bIsSimulating) {
		if (IsValidLowLevel() && !IS_TRANSIENT(this))
		{
			USingletonLibrary::GetSingleton()->LevelMap_ = this;
			USingletonLibrary::PrintToLog(this, "EndPIE", "");
		}
	});
#endif  //WITH_EDITOR [Editor]

	// Initialize root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetRelativeScale3D(FVector(5.f, 5.f, 1.f));

	// Initialize background mesh component
	BackgroundMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackgroundMeshComponent"));
	BackgroundMeshComponent->SetupAttachment(RootComponent);
	BackgroundMeshComponent->bAbsoluteScale = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BackgroundMeshFinder(TEXT("/Game/Bomber/Assets/Meshes/BackgroundMesh"));
	if (BackgroundMeshFinder.Succeeded())
	{
		BackgroundMesh = BackgroundMeshFinder.Object;  // Default mesh of the BackgroundMeshComponent
	}

	// Initialize platform component
	PlatformComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlatformComponent"));
	PlatformComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FClassFinder<AActor> PlatformClassFinder(TEXT("/Game/Bomber/Assets/PlatformAsset"));
	if (PlatformClassFinder.Succeeded())
	{
		PlatformClass = PlatformClassFinder.Class;  // Default class of the PlatformComponent
	}
}

TSet<FCell> AGeneratedMap::IntersectionCellsByTypes_Implementation(
	const TSet<FCell>& Cells,
	const int32& ActorsTypesBitmask,
	const AMyCharacter* ExcludePlayer) const
{
	return Cells;
}

TSet<FCell> AGeneratedMap::GetSidesCells_Implementation(
	const FCell& Cell,
	const int32 SideLength,
	const EPathTypesEnum Pathfinder) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
}

void AGeneratedMap::AddActorOnMapByObj(const FCell& Cell, AActor* UpdateActor)
{
	if (IS_VALID(UpdateActor) == false			// Updating actor is not valid
		|| GridArray_.Contains(Cell) == false)  // Not existing cell
	{
		return;
	}

	// Add actor to specific array
	AMyCharacter* const UpdateCharacter = Cast<AMyCharacter>(UpdateActor);
	if (UpdateCharacter != nullptr)  // if it is character, add to array of characters
	{
		CharactersOnMap.Add(UpdateCharacter);  // Add this character
	}
	else  // else if this class can be added
		if (USingletonLibrary::GetSingleton()->ActorTypesByClasses.FindKey(UpdateActor->GetClass()) != nullptr)
	{
		RemoveActorFromGridArray(UpdateActor);
		GridArray_.Add(Cell, UpdateActor);  // Add this actor to his new cell
	}

	UpdateActor->GetRootComponent()->SetAbsolute(false, false, true);

	// Locate actor on cell
	FRotator ActorRotation{GetActorRotation()};
	ActorRotation.Yaw += FMath::RandRange(int32(1), int32(4)) * 90.f;
	const FVector ActorLocation{Cell.Location.X, Cell.Location.Y, Cell.Location.Z + 100.f};
	const FVector Scale{1.f, 1.f, 1.f};
	UpdateActor->SetActorTransform(FTransform(ActorRotation, ActorLocation, Scale));

	// Attach to the Level Map actor
	UpdateActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	USingletonLibrary::PrintToLog(UpdateActor, "AddActorOnMapByObj \t ADDED:", Cell.Location.ToString());
}

void AGeneratedMap::DestroyActorsFromMap_Implementation(const TSet<FCell>& Keys)
{
	USingletonLibrary::PrintToLog(this, "DestroyActorsFromMap \t Keys will be destroyed:", FString::FromInt(Keys.Num()));
}

void AGeneratedMap::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IS_TRANSIENT(this) == true)  // the level map is transient
	{
		return;
	}

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::GetSingleton()->LevelMap_ = this;

	// fix null keys
	CharactersOnMap.Compact();
	CharactersOnMap.Shrink();

	// Actors generation
	GenerateLevelActors();
}

void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	if (IS_TRANSIENT(this) == true)  // the level map is transient
	{
		return;
	}
	// Update the constructed LevelMap obj;
	USingletonLibrary::GetSingleton()->LevelMap_ = this;

	// Update the background static mesh
	if (BackgroundMesh != nullptr)
	{
		BackgroundMeshComponent->SetStaticMesh(BackgroundMesh);
	}

	// Create the platform blueprint child actor
	if (PlatformClass != nullptr)
	{
		PlatformComponent->SetChildActorClass(PlatformClass);
		PlatformComponent->CreateChildActor();
	}

	// Align the Transform
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	SetActorLocation(GetActorLocation().GridSnap(USingletonLibrary::GetGridSize()));
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

	// Loopy cell-filling of the grid array
	GridArray_.Empty();
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			FVector FoundVector(X, Y, 0.f);
			// Calculate a length of iteration cell
			FoundVector *= USingletonLibrary::GetGridSize();
			// Locate the cell relative to the Level Map
			FoundVector += GetActorLocation();
			// Subtract the deviation from the center
			FoundVector -= GetActorScale3D() / 2 * USingletonLibrary::GetGridSize();
			// Snap to the cell
			FoundVector = FoundVector.GridSnap(USingletonLibrary::GetGridSize());
			// Cell was found, add rotated cell to the array
			const FCell FoundCell = USingletonLibrary::CalculateVectorAsRotatedCell(FoundVector, 1.f);
			GridArray_.Add(FoundCell);
		}
	}

#if WITH_EDITOR						 // [PIE] Map's text renders
	if (IS_PIE(GetWorld()) == true)  // For editor only
	{
		// Show cell coordinated of the Grid array
		USingletonLibrary::ClearOwnerTextRenders(this);
		if (bShouldShowRenders == true)
		{
			TArray<FCell> ArrayRenders;
			GridArray_.GetKeys(ArrayRenders);
			const TSet<FCell> SetRenders(ArrayRenders);
			USingletonLibrary::AddDebugTextRenders(this, SetRenders);
		}

		// Preview generation
		GenerateLevelActors();
	}
#endif  // WITH_EDITOR [Editor]
}

#if WITH_EDITOR  // [PIE] Destroyed()
void AGeneratedMap::Destroyed()
{
	if (IS_PIE(GetWorld()) == true		 // For editor only
		&& IS_TRANSIENT(this) == false)  // Component is not transient
	{
		DestroyAttachedActors();

		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.Clear();

		// Remove all elements of arrays
		GridArray_.Empty();
		CharactersOnMap.Empty();

		// Remove from the singleton
		USingletonLibrary::GetSingleton()->LevelMap_ = nullptr;
	}

	// Call the base class version
	Super::Destroyed();
}
#endif  //WITH_EDITOR [PIE]

void AGeneratedMap::DestroyAttachedActors(bool bIsEditorOnlyActors)
{
#if WITH_EDITOR  // [PIE]
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	if (AttachedActors.Num() == 0)
	{
		return;
	}

	for (int32 i = AttachedActors.Num() - 1; i >= 0; --i)
	{
		if (bIsEditorOnlyActors == false		   // Should destroy all actors
			|| AttachedActors[i]->IsEditorOnly())  // Should destroy editor-only actors
		{
			AttachedActors[i]->Destroy();
		}
	}

	// After destroying PIE actors and before their generation,
	// calling to updating of all dragged to the Level Map actors
	if (IS_PIE(GetWorld())  //
		&& bIsEditorOnlyActors)
	{
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.Broadcast();
	}  // [PIE]

#endif  //WITH_EDITOR [Editor]
}

void AGeneratedMap::RemoveActorFromGridArray(const AActor* Actor)
{
	const FCell* Cell = GridArray_.FindKey(Actor);
	if (Cell == nullptr)  // The actor was not found on anyone cell
	{
		return;
	}

	GridArray_.Add(*Cell);  // remove this actor from this cell
	USingletonLibrary::PrintToLog(this, "RemoveActorFromGridArray", Actor->GetName());
}

void AGeneratedMap::GenerateLevelActors_Implementation()
{
	if (GridArray_.Num() == 0)  // Is no cell to generate an actor
	{
		return;
	}

#if WITH_EDITOR  // [Editor]
	//  Destroy editor-only actors that were spawned in the PIE
	DestroyAttachedActors(true);
#endif  // [Editor]

	TArray<FCell> CellsArray;
	GridArray_.GetKeys(CellsArray);

	const FIntVector MapScale(GetActorScale3D());
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			const FCell CellIt = CellsArray[MapScale.X * Y + X];
			const AActor** ActorIt = GridArray_.Find(CellIt);
			if (ActorIt != nullptr				// Was found actor
				&& IS_VALID(*ActorIt) == true)  // and this actor is valid
			{
				USingletonLibrary::PrintToLog(this, "GenerateLevelActors \t The actor on the cell has already existed", (*ActorIt)->GetName());
				continue;
			}

			// --- The spawn part ---

			// In case all next conditions will be false
			EActorTypeEnum ActorTypeToSpawn = EActorTypeEnum::None;

			// Wall condition
			if (X % 2 == 1 && Y % 2 == 1)
			{
				ActorTypeToSpawn = EActorTypeEnum::Wall;
			}

			// Player condition
			const bool bIsCornerX = (X == 0 || X == MapScale.X - 1);
			const bool bIsCornerY = (Y == 0 || Y == MapScale.Y - 1);
			if (bIsCornerX && bIsCornerY)
			{
				ActorTypeToSpawn = EActorTypeEnum::Player;
			}

			// Box condition
			if (ActorTypeToSpawn == EActorTypeEnum::None				  // all previous conditions are false
				&& FMath::RandRange(int32(0), int32(99)) < BoxesChance_   // Chance of boxes
				&& (!bIsCornerX && X != 1 && X != MapScale.X - 2		  // X corner zone
					   || !bIsCornerY && Y != 1 && Y != MapScale.Y - 2))  // Y corner zone
			{
				ActorTypeToSpawn = EActorTypeEnum::Box;
			}

			// --- The spawn part ---
			const auto ActorClass = USingletonLibrary::FindClassByActorType(EActorTypeEnum(ActorTypeToSpawn));
			if (ActorClass != nullptr)  // There is type to spawn
			{
				AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, CellIt.Location, FRotator::ZeroRotator);

#if WITH_EDITOR									 // [PIE]
				if (IS_PIE(GetWorld())			 // PIE only
					&& SpawnedActor != nullptr)  // Successfully spawn
				{
					// If PIE world, mark this spawned actor as bIsEditorOnlyActor
					SpawnedActor->bIsEditorOnlyActor = true;
					USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.RemoveAll(SpawnedActor);
				}
#endif		   // WITH_EDITOR [PIE]
			}  // End of the spawn part
		}	  // X iterations
	}		   // Y iterations
}
