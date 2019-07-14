// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "ConstructorHelpers.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetRelativeScale3D(FVector(5.f, 5.f, 1.f));
	RootComponent->SetMobility(EComponentMobility::Movable);
	//RootComponent->SetAbsolute(true, true, true);

	// Find blueprints
	const TArray<TCHAR*> Paths{
		TEXT("/Game/Bomber/Assets/Wall"),		  //EPathTypesEnum::Wall
		TEXT("/Game/Bomber/Assets/Box"),		  //EPathTypesEnum::Box
		TEXT("/Game/Bomber/Blueprints/BpBomb"),   //EPathTypesEnum::Bomb
		TEXT("/Game/Bomber/Blueprints/BpItem"),   //EPathTypesEnum::Item
		TEXT("/Game/Bomber/Blueprints/BpPlayer")  //EPathTypesEnum::Player
	};
	for (int32 i = 0; i < Paths.Num(); ++i)
	{
		ConstructorHelpers::FClassFinder<AActor> ClassFinder(Paths[i]);
		TypesByClassesMap.Add(
			EActorTypeEnum(1 << i), (ClassFinder.Succeeded() ? ClassFinder.Class : nullptr));
	}

// Should not call OnConstruction on drag events
#if WITH_EDITOR
	bRunConstructionScriptOnDrag = false;
#endif  //WITH_EDITOR [Dev]
}

TSet<FCell> AGeneratedMap::IntersectionCellsByTypes_Implementation(
	const TSet<FCell>& Cells,
	const uint8& ActorsTypesBitmask,
	const AMyCharacter* ExcludePlayer) const
{
	TSet<FCell> FoundedLocations;
	return FoundedLocations;
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
		if (TypesByClassesMap.FindKey(UpdateActor->GetClass()) != nullptr)
	{
		const FCell* CellOfExistingActor = GridArray_.FindKey(UpdateActor);
		if (CellOfExistingActor != nullptr && CellOfExistingActor->Location != Cell.Location)
		{
			GridArray_.Add(*CellOfExistingActor);  // remove this actor from previous cell
			UE_LOG_STR(UpdateActor, "AddActorOnMapByObj", "Removed existed actor from cell");
		}
		GridArray_.Add(Cell, UpdateActor);  // Add this actor to his cell
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

	UE_LOG_STR(UpdateActor, "AddActorOnMapByObj \t ADDED:", Cell.Location.ToString());
}

void AGeneratedMap::DestroyActorsFromMap_Implementation(const TSet<FCell>& Keys)
{
	UE_LOG_STR(this, "DestroyActorsFromMap \t Keys will be destroyed:", FString::FromInt(Keys.Num()));
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	// Update UEDPIE_LevelMap obj;
	USingletonLibrary::GetSingleton()->LevelMap_ = this;

	// fix null keys
	CharactersOnMap.Compact();
	CharactersOnMap.Shrink();

	// Loopy walls generation
	GenerateLevelMap((uint8)EActorTypeEnum::Wall & (uint8)EActorTypeEnum::Box & (uint8)EActorTypeEnum::Player);
}

void AGeneratedMap::OnConstruction(const FTransform& Transform)
{
	if (IS_TRANSIENT(this) == true)  // the level map is transient
	{
		return;
	}

	// Align the Transform
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	SetActorLocation(GetActorLocation().GridSnap(USingletonLibrary::GetFloorLength()));
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
	UE_LOG_STR(this, "OnConstruction \t Scale:", MapScale.ToString());

	// Loopy cell-filling of the grid array
	GridArray_.Empty();
	for (int32 x = 0; x < MapScale.X; ++x)
	{
		for (int32 y = 0; y < MapScale.Y; ++y)
		{
			FCell CellIt;
			// Calculate a length of iteration cell
			CellIt.Location = FVector(x, y, 0.f) * USingletonLibrary::GetFloorLength();
			// Locate the cell relative to the Level Map
			CellIt.Location += GetActorLocation();
			// Subtract the deviation from the center
			CellIt.Location -= (GetActorScale3D() / 2 * USingletonLibrary::GetFloorLength());
			// Snap to the cell
			CellIt.Location = CellIt.Location.GridSnap(USingletonLibrary::GetFloorLength());
			// Rotate the cell around center
			const FVector RelativePos{(CellIt.Location - GetActorLocation())};
			CellIt.Location += RelativePos.RotateAngleAxis(GetActorRotation().Yaw, FVector(0, 0, 1)) - RelativePos;
			// Round the cell
			CellIt.Location = FVector(FIntVector(CellIt.Location));

			GridArray_.Add(CellIt, nullptr);
		}
	}
#if WITH_EDITOR						 //[PIE-DEBUG]
	if (IS_PIE(GetWorld()) == true)  // For editor only
	{
		USingletonLibrary::ClearOwnerTextRenders(this);
		TArray<FCell> ArrayRenders;
		GridArray_.GetKeys(ArrayRenders);
		const TSet<FCell> SetRenders(ArrayRenders);
		USingletonLibrary::AddDebugTextRenders(this, SetRenders);
	}
#endif  // WITH_EDITOR [PIE-DEBUG]
}

#if WITH_EDITOR
void AGeneratedMap::Destroyed()
{
	if (IS_PIE(GetWorld()) == true		 // For editor only
		&& IS_TRANSIENT(this) == false)  // Component is not transient
	{
		// Destroy all attached actors
		TArray<AActor*> AttachedActors;
		GetAttachedActors(AttachedActors);
		if (AttachedActors.Num() > 0)
		{
			for (int32 i = AttachedActors.Num() - 1; i >= 0; --i)
			{
				AttachedActors[i]->Destroy();
			}
			UE_LOG_STR(this, "ClearLevelMap \t Actors removed:", FString::FromInt(AttachedActors.Num()));
		}

		// Remove all elements of arrays
		GridArray_.Empty();
		CharactersOnMap.Empty();

		// Clear Singleton properties
		USingletonLibrary* const Singleton = USingletonLibrary::GetSingleton();
		if (Singleton != nullptr)  // Singleton is not null
		{
			Singleton->LevelMap_ = nullptr;
			Singleton->OnActorsUpdatedDelegate.Clear();
			UE_LOG_STR(this, "[PIE]Destroyed", "Cleared the Singleton's properties");
		}
	}

	// Call the base class version
	Super::Destroyed();
}

#endif  //WITH_EDITOR [PIE]

void AGeneratedMap::GenerateLevelMap_Implementation(const uint8& ActorsTypesBitmask)
{
	//Generation on free from actors cells
}