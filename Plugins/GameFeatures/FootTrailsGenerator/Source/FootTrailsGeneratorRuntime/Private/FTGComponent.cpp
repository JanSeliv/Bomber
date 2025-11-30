// Copyright (c) Yevhenii Selivanov

#include "FTGComponent.h"

// FTG
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
	return UMyPrimaryDataAsset::GetOrLoadOnce(FootTrailsDataAsset);
}

// Guarantees that the data asset is loaded, otherwise, it will crash
const UFTGDataAsset& UFTGComponent::GetFootTrailsDataAssetChecked() const
{
	const UFTGDataAsset* InFootTrailsDataAsset = GetFootTrailsDataAsset();
	checkf(InFootTrailsDataAsset, TEXT("%s: 'FootTrailsDataAsset' is not set"), *FString(__FUNCTION__));
	return *InFootTrailsDataAsset;
}

// Returns the random foot trail instance for given types
const UStaticMesh* UFTGComponent::GetRandomMesh(EFTGTrailType FootTrailType) const
{
	TArray<const UStaticMesh*> MatchingMeshes;
	const EBmrLevelType CurrentLevelType = UBmrBlueprintFunctionLibrary::GetLevelType();
	for (const TTuple<FFTGArchetype, TObjectPtr<UStaticMesh>>& It : FootTrailInstances)
	{
		if (It.Key.FootTrailType == FootTrailType && It.Key.LevelType == CurrentLevelType)
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

	UMyPrimaryDataAsset::ResetDataAsset(FootTrailsDataAsset);

	Super::EndPlay(EndPlayReason);
}

// Loads all foot trails archetypes
void UFTGComponent::InitOnce()
{
	const UDataTable* FootTrailsDT = GetFootTrailsDataAssetChecked().GetFootTrailsDataTable();
	if (!ensureMsgf(FootTrailsDT, TEXT("%s: 'FootTrailsDT' is not set"), *FString(__FUNCTION__))
	    || !FootTrailInstances.IsEmpty())
	{
		// is already initialized
		return;
	}

	InstancedStaticMeshActor = GetWorld()->SpawnActor<AInstancedStaticMeshActor>();
	checkf(InstancedStaticMeshActor, TEXT("%s: ERROR: 'InstancedStaticMeshActor' was not spawned!"), *FString(__FUNCTION__));

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

	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("%s: ERROR: 'Owner' is null!"), *FString(__FUNCTION__));
	const FRotator OwnerLootAtRot = Owner->GetActorRotation();
	CellRotation += UBmrCellUtilsLibrary::GetCellYawDegree();
	const FRotator CellRot(OwnerLootAtRot.Pitch, CellRotation, OwnerLootAtRot.Roll);
	const FTransform CellTransform(CellRot, Cell, FVector::OneVector);

	InstancedStaticMeshActor->SpawnInstanceByMesh(CellTransform, FootTrailMesh);
}
