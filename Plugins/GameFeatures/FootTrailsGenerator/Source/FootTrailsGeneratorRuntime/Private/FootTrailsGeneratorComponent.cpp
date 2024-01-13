// Copyright (c) Yevhenii Selivanov

#include "FootTrailsGeneratorComponent.h"
//---
#include "FootTrailsDataAsset.h"
#include "InstancedStaticMeshActor.h"
#include "MyDataTable/MyDataTable.h"
#include "Structures/Cell.h"
#include "UtilityLibraries/CellsUtilsLibrary.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
//---
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(FootTrailsGeneratorComponent)

// Sets default values for this component's properties
UFootTrailsGeneratorComponent::UFootTrailsGeneratorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Guarantees that the data asset is loaded, otherwise, it will crash
const UFootTrailsDataAsset& UFootTrailsGeneratorComponent::GetFootTrailsDataAssetChecked() const
{
	const UFootTrailsDataAsset* FootTrailsDataAsset = GetFootTrailsDataAsset();
	checkf(FootTrailsDataAsset, TEXT("%s: 'FootTrailsDataAssetInternal' is not set"), *FString(__FUNCTION__));
	return *FootTrailsDataAsset;
}

// Returns the random foot trail instance for given types
const UStaticMesh* UFootTrailsGeneratorComponent::GetRandomMesh(EFootTrailType FootTrailType) const
{
	TArray<const UStaticMesh*> MatchingMeshes;
	const ELevelType CurrentLevelType = UMyBlueprintFunctionLibrary::GetLevelType();
	for (const TTuple<FFootTrailArchetype, TObjectPtr<UStaticMesh>>& It : FootTrailInstancesInternal)
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
void UFootTrailsGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	Init();
}

// Called when the game ends
void UFootTrailsGeneratorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InstancedStaticMeshActorInternal)
	{
		InstancedStaticMeshActorInternal->Destroy();
		InstancedStaticMeshActorInternal = nullptr;
	}

	FootTrailInstancesInternal.Empty();

	Super::EndPlay(EndPlayReason);
}

// Loads all foot trails archetypes
void UFootTrailsGeneratorComponent::Init()
{
	const UDataTable* FootTrailsDT = GetFootTrailsDataAssetChecked().GetFootTrailsDataTable();
	if (!ensureMsgf(FootTrailsDT, TEXT("%s: 'FootTrailsDT' is not set"), *FString(__FUNCTION__))
		|| !FootTrailInstancesInternal.IsEmpty())
	{
		// is already initialized
		return;
	}

	InstancedStaticMeshActorInternal = GetWorld()->SpawnActor<AInstancedStaticMeshActor>();
	checkf(InstancedStaticMeshActorInternal, TEXT("%s: ERROR: 'InstancedStaticMeshActor' was not spawned!"), *FString(__FUNCTION__));

	TMap<FName, FFootTrailArchetype> FootTrailsRows;
	UMyDataTable::GetRows(*FootTrailsDT, FootTrailsRows);
	for (const TTuple<FName, FFootTrailArchetype>& FootTrailsRowIt : FootTrailsRows)
	{
		const FFootTrailArchetype& ArchetypeIt = FootTrailsRowIt.Value;
		if (ArchetypeIt.Mesh.IsNull())
		{
			// skip empty rows
			continue;
		}

		FootTrailInstancesInternal.Emplace(ArchetypeIt, ArchetypeIt.Mesh.LoadSynchronous());
	}
}

// Spawns given Foot Trail by its type on the specified cell
void UFootTrailsGeneratorComponent::SpawnFootTrail(EFootTrailType FootTrailType, const FCell& Cell, float CellRotation)
{
	const UStaticMesh* FootTrailMesh = GetRandomMesh(FootTrailType);
	if (!FootTrailMesh
		|| !ensureMsgf(FootTrailType != EFootTrailType::None, TEXT("%s: 'FootTrailType' is none"), *FString(__FUNCTION__))
		|| !ensureMsgf(InstancedStaticMeshActorInternal, TEXT("%s: 'InstancedStaticMeshActor' is not valid"), *FString(__FUNCTION__)))
	{
		return;
	}

	const AActor* Owner = GetOwner();
	checkf(Owner, TEXT("%s: ERROR: 'Owner' is null!"), *FString(__FUNCTION__));
	const FRotator OwnerLootAtRot = Owner->GetActorRotation();
	CellRotation += UCellsUtilsLibrary::GetCellYawDegree();
	const FRotator CellRot(OwnerLootAtRot.Pitch, CellRotation, OwnerLootAtRot.Roll);
	const FTransform CellTransform(CellRot, Cell, FVector::OneVector);

	InstancedStaticMeshActorInternal->SpawnInstanceByMesh(CellTransform, FootTrailMesh);
}
