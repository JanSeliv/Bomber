// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/AIUtilsLibrary.h"
//---
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NavMesh/NavMeshBoundsVolume.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(AIUtilsLibrary)

// Finds and transform Navigation Mesh
void UAIUtilsLibrary::RebuildNavMesh(UObject* WorldContextObject, const FTransform& Transform)
{
	ANavMeshBoundsVolume* NavMesh = Cast<ANavMeshBoundsVolume>(UGameplayStatics::GetActorOfClass(WorldContextObject, ANavMeshBoundsVolume::StaticClass()));
	if (!ensureMsgf(NavMesh, TEXT("ASSERT: [%i] %s:\nNo NavMeshBoundsVolume found on level, it has to be preplaced anywhere first since can not be spawned in runtime because of Brush!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	// Change transform
	ensureMsgf(NavMesh->GetRootComponent()->Mobility == EComponentMobility::Movable, TEXT("ASSERT: [%i] %s:\n'NavMesh' is not movable!"), __LINE__, *FString(__FUNCTION__));
	NavMesh->SetActorTransform(Transform);

	// Update navmesh
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(WorldContextObject);
	if (CanRebuildNavMesh(NavSys))
	{
		NavSys->OnNavigationBoundsUpdated(NavMesh);
	}
}

// Returns true if the Navigation Mesh can be rebuilt
bool UAIUtilsLibrary::CanRebuildNavMesh(const UNavigationSystemV1* NavSys)
{
	if (!ensureMsgf(NavSys, TEXT("ASSERT: [%i] %s:\n'NavSys' is null!"), __LINE__, *FString(__FUNCTION__))
		|| !ensureMsgf(!NavSys->IsNavigationSystemStatic(), TEXT("ASSERT: [%i] %s:\n'Navigation System' is configured as static!"), __LINE__, *FString(__FUNCTION__)))
	{
		return false;
	}

	const ANavigationData* NavData = Cast<ANavigationData>(NavSys->GetMainNavData());
	return ensureMsgf(NavData, TEXT("ASSERT: [%i] %s:\n'NavData' is null!"), __LINE__, *FString(__FUNCTION__))
		&& ensureMsgf(NavData->GetRuntimeGenerationMode() == ERuntimeGenerationType::Dynamic, TEXT("ASSERT: [%i] %s:\n'RuntimeGeneration' has to be 'Dynamic' in the Project Settings!"), __LINE__, *FString(__FUNCTION__))
		&& ensureMsgf(NavData->IsRegistered(), TEXT("ASSERT: [%i] %s:\n'NavData' is not registered!"), __LINE__, *FString(__FUNCTION__));
}
