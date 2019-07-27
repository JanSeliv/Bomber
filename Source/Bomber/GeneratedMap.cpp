// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneratedMap.h"

#include "Bomber.h"
#include "Cell.h"
#include "Math/UnrealMathUtility.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AGeneratedMap::AGeneratedMap()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Should not call OnConstruction on drag events
#if WITH_EDITOR  // [Editor] bRunConstructionScriptOnDrag
	bRunConstructionScriptOnDrag = false;
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
		BackgroundMeshComponent->SetStaticMesh(BackgroundMeshFinder.Object);
	}

	// Initialize platform component
	auto PlatformComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlatformComponent"));
	PlatformComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FClassFinder<AActor> PlatformClassFinder(TEXT("/Game/Bomber/Assets/PlatformAsset"));
	if (PlatformClassFinder.Succeeded())
	{
		PlatformComponent->SetChildActorClass(PlatformClassFinder.Class);
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
		const FCell* CellOfExistingActor = GridArray_.FindKey(UpdateActor);
		if (CellOfExistingActor != nullptr && !(*CellOfExistingActor == Cell))
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

	// Boxes generation
	GenerateLevelActors(1 << int32(EActorTypeEnum::Box), FCell::ZeroCell);
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

	// Loopy cell-filling of the grid array
	GridArray_.Empty();
	for (int32 Y = 0; Y < MapScale.Y; ++Y)
	{
		for (int32 X = 0; X < MapScale.X; ++X)
		{
			FVector FoundVector(X, Y, 0.f);
			// Calculate a length of iteration cell
			FoundVector *= USingletonLibrary::GetFloorLength();
			// Locate the cell relative to the Level Map
			FoundVector += GetActorLocation();
			// Subtract the deviation from the center
			FoundVector -= (GetActorScale3D() / 2 * USingletonLibrary::GetFloorLength());
			// Snap to the cell
			FoundVector = FoundVector.GridSnap(USingletonLibrary::GetFloorLength());
			// Rotate the cell around center
			const FVector RelativePos(FoundVector - GetActorLocation());
			FoundVector += RelativePos.RotateAngleAxis(GetActorRotation().Yaw, FVector(0, 0, 1)) - RelativePos;
			// Cell was found, add it to the array
			const FCell FoundCell(FoundVector);
			GridArray_.Add(FoundCell);
		}
	}

#if WITH_EDITOR						 // [PIE] Map's text renders
	if (IS_PIE(GetWorld()) == true)  // For editor only
	{
		// Destroy editor-only actors that were spawned in the PIE
		DestroyAttachedActors(true);

		// Show cell coordinated of the Grid array
		USingletonLibrary::ClearOwnerTextRenders(this);
		if (bShouldShowRenders == true)
		{
			TArray<FCell> ArrayRenders;
			GridArray_.GetKeys(ArrayRenders);
			const TSet<FCell> SetRenders(ArrayRenders);
			USingletonLibrary::AddDebugTextRenders(this, SetRenders);
		}
	}

#endif  // WITH_EDITOR [Editor]

	// After destroying PIE actors and before their generation,
	// calling to updating of all dragged to the Level Map actors
	USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.Broadcast();

	// Walls and Players generation
	GenerateLevelActors(1 << int32(EActorTypeEnum::Wall) | 1 << int32(EActorTypeEnum::Player), FCell::ZeroCell);
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

void AGeneratedMap::DestroyAttachedActors(bool bIsEditorOnlyActors) const
{
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
			UE_LOG_STR(AttachedActors[i], "DestroyAttachedActors", "Will be removed")
			AttachedActors[i]->Destroy();
		}
	}
}

#endif  //WITH_EDITOR [PIE]

void AGeneratedMap::GenerateLevelActors_Implementation(const int32& ActorsTypesBitmask, const FCell& Cell)
{
	const auto ActorClass = USingletonLibrary::FindClassByActorType(EActorTypeEnum(ActorsTypesBitmask));
	if (ActorClass == nullptr)
	{
		return;
	}

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, Cell.Location, FRotator::ZeroRotator);

// If PIE world, mark this spawned actor as bIsEditorOnlyActor
#if WITH_EDITOR  // [PIE]
	if (IS_PIE(GetWorld()))
	{
		SpawnedActor->bIsEditorOnlyActor = true;
	}
#endif  // WITH_EDITOR [PIE]
}