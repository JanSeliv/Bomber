// Copyright (c) Yevhenii Selivanov

#include "FTGComponent.h"

// FTG
#include "DalSubsystem.h"
#include "FTGDataAsset.h"

// Bomber
#include "Actors/BmrGeneratedMap.h"
#include "InstancedStaticMeshActor.h"
#include "MyDataTable/MyDataTable.h"
#include "Structures/BmrCell.h"
#include "UtilityLibraries/BmrBlueprintFunctionLibrary.h"
#include "UtilityLibraries/BmrCellUtilsLibrary.h"

// UE
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FTGComponent)

// Sets default values for this component's properties
UFTGComponent::UFTGComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Returns the data asset that contains all the assets and tweaks of Foot Trails game feature
const UFTGDataAsset* UFTGComponent::GetFootTrailsDataAsset() const
{
	return UDalSubsystem::GetDataAsset<UFTGDataAsset>();
}

// Returns the random foot trail instance for given types
const UStaticMesh* UFTGComponent::GetRandomMesh(EFTGTrailType FootTrailType) const
{
	TArray<const UStaticMesh*> MatchingMeshes;
	for (const TTuple<FFTGArchetype, TObjectPtr<UStaticMesh>>& It : FootTrailInstances)
	{
		if (It.Key.FootTrailType == FootTrailType)
		{
			MatchingMeshes.Emplace(It.Value);
		}
	}

	const int32 Index = FMath::RandRange(0, MatchingMeshes.Num() - 1);
	return MatchingMeshes.IsValidIndex(Index) ? MatchingMeshes[Index] : nullptr;
}

// Called when the game starts
void UFTGComponent::BeginPlay()
{
	Super::BeginPlay();

	InitOnce();
}

// Called when the game ends
void UFTGComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InstancedStaticMeshActor)
	{
		InstancedStaticMeshActor->Destroy();
		InstancedStaticMeshActor = nullptr;
	}

	FootTrailInstances.Empty();

	Super::EndPlay(EndPlayReason);
}

// Loads all foot trails archetypes
void UFTGComponent::InitOnce()
{
	if (!FootTrailInstances.IsEmpty())
	{
		// is already initialized
		return;
	}

	UDalSubsystem::Get().ListenForDataAsset<UFTGDataAsset>(this, [this](const UFTGDataAsset& DataAsset)
	{
		const UDataTable* FootTrailsDT = DataAsset.GetFootTrailsDataTable();
		if (!ensureMsgf(FootTrailsDT, TEXT("UFTGComponent::InitOnce: 'FootTrailsDT' is not set")))
		{
			return;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.OverrideLevel = GetWorld()->PersistentLevel; // Always keep new objects on Persistent level
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.bNoFail = true;
#if WITH_EDITORONLY_DATA
		SpawnParameters.bCreateActorPackage = false; // Do not bake this runtime actor into World Partition level
#endif

		InstancedStaticMeshActor = GetWorld()->SpawnActor<AInstancedStaticMeshActor>(SpawnParameters);
		checkf(InstancedStaticMeshActor, TEXT("UFTGComponent::InitOnce: ERROR: 'InstancedStaticMeshActor' was not spawned!"));

		TMap<FName, FFTGArchetype> FootTrailsRows;
		UMyDataTable::GetRows(*FootTrailsDT, FootTrailsRows);
		for (const TTuple<FName, FFTGArchetype>& FootTrailsRowIt : FootTrailsRows)
		{
			const FFTGArchetype& ArchetypeIt = FootTrailsRowIt.Value;
			if (ArchetypeIt.Mesh.IsNull())
			{
				// skip empty rows
				continue;
			}

			FootTrailInstances.Emplace(ArchetypeIt, ArchetypeIt.Mesh.LoadSynchronous());
		}

		// Generate first trails and bind to further regenerations
		BPGenerateFootTrails();
		ABmrGeneratedMap::Get().OnGeneratedLevelActors.AddUniqueDynamic(this, &ThisClass::BPGenerateFootTrails);
	});
}

// Spawns given Foot Trail by its type on the specified cell
void UFTGComponent::SpawnFootTrail(EFTGTrailType FootTrailType, const FBmrCell& Cell, float CellRotation)
{
	const UStaticMesh* FootTrailMesh = GetRandomMesh(FootTrailType);
	if (!FootTrailMesh
	    || !ensureMsgf(FootTrailType != EFTGTrailType::None, TEXT("%s: 'FootTrailType' is none"), *FString(__FUNCTION__))
	    || !ensureMsgf(InstancedStaticMeshActor, TEXT("%s: 'InstancedStaticMeshActor' is not valid"), *FString(__FUNCTION__)))
	{
		return;
	}

	const FRotator OwnerLootAtRot = ABmrGeneratedMap::Get().GetActorRotation();
	CellRotation += UBmrCellUtilsLibrary::GetCellYawDegree();
	const FRotator CellRot(OwnerLootAtRot.Pitch, CellRotation, OwnerLootAtRot.Roll);
	const FTransform CellTransform(CellRot, Cell, FVector::OneVector);

	InstancedStaticMeshActor->SpawnInstanceByMesh(CellTransform, FootTrailMesh);
}
