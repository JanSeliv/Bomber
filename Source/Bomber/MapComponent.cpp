// Fill out your copyright notice in the Description page of Project Settings.

#include "MapComponent.h"

#include "Bomber.h"
#include "BoxActor.h"
#include "GeneratedMap.h"
#include "SingletonLibrary.h"

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UMapComponent::UpdateSelfOnMap()
{
	UWorld* const World = GetWorld();
	if (World == nullptr									  // World is null
		|| IS_VALID(GetOwner()) == false					  // owner is not valid
		|| USingletonLibrary::GetLevelMap(World) == nullptr)  // levelMap is null
	{
		return;
	}
	UE_LOG_STR(GetOwner(), "UpdateSelfOnMap", "Starts updating");
	// Find new Location at dragging and update-delegate
	Cell = FCell(GetOwner());

	USingletonLibrary::GetLevelMap(World)->AddActorOnMapByObj(Cell, GetOwner());

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // for editor only
	{
		// Remove all text renders of the Owner
		USingletonLibrary::ClearOwnerTextRenders(GetOwner());

		// Update AI renders after adding obj to map
		USingletonLibrary::GetSingleton()->BroadcastAiUpdating(GetOwner());
	}
#endif  //WITH_EDITOR [PIE]
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (IS_VALID(GetOwner()) == false)  // owner is not valid
	{
		return;
	}

	UE_LOG_STR(GetOwner(), "OnComponentCreated", "Starts creating");

	// Disable tick
	GetOwner()->SetActorTickEnabled(false);

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true						  // for editor only
		&& USingletonLibrary::GetSingleton() != nullptr)  // Singleton is valid
	{
		// Should not call OnConstruction on drag events
		GetOwner()->bRunConstructionScriptOnDrag = false;

		// Binds to updating actors on the Level Map
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);

		// Binds to updating AI renders on owner destroying
		GetOwner()->OnDestroyed.AddDynamic(USingletonLibrary::GetSingleton(), &USingletonLibrary::BroadcastAiUpdating);
	}
#endif  //WITH_EDITOR [PIE]
}
