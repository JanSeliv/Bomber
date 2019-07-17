// Fill out your copyright notice in the Description page of Project Settings.

#include "MapComponent.h"

#include "Bomber.h"
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

	// Find new Location at dragging and update-delegate
	Cell = FCell(GetOwner());

	// Owner updating
	UE_LOG_STR(GetOwner(), "UpdateSelfOnMap", "-> \t AddActorOnMapByObj");
	USingletonLibrary::GetLevelMap(World)->AddActorOnMapByObj(Cell, GetOwner());

	// Rerun owner's construction scripts
	UE_LOG_STR(GetOwner(), "UpdateSelfOnMap", "-> \t RerunConstructionScripts");
	GetOwner()->RerunConstructionScripts();
}

void UMapComponent::OnMapComponentConstruction()
{
	if (IS_VALID(GetOwner()) == false					  // The owner is not valid
		|| USingletonLibrary::GetSingleton() == nullptr)  // The Singleton is null
	{
		return;
	}

	// Binds to updating non-generated actors on the Level Map
	USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddUniqueDynamic(this, &UMapComponent::UpdateSelfOnMap);

#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // PIE only
	{
		// Remove all text renders of the Owner
		UE_LOG_STR(GetOwner(), "[PIE]OnMapComponentConstruction", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(GetOwner());

		// Binds to updating AI renders on owner destroying
		GetOwner()->OnDestroyed.AddUniqueDynamic(USingletonLibrary::GetSingleton(), &USingletonLibrary::BroadcastAiUpdating);

		// Update AI renders after adding obj to map
		UE_LOG_STR(GetOwner(), "[PIE]OnMapComponentConstruction", "-> \t BroadcastAiUpdating");
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
	UE_LOG_STR(GetOwner(), "OnComponentCreated", "Set's defaults");

	// Disable the tick
	GetOwner()->SetActorTickEnabled(false);

	// Set the movable mobility for in-game attaching
	if (GetOwner()->GetRootComponent() != nullptr)
	{
		GetOwner()->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

// Should not call OnConstruction on drag events
#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true)  // PIE only
	{
		GetOwner()->bRunConstructionScriptOnDrag = false;
	}
#endif  //WITH_EDITOR [PIE]
}
