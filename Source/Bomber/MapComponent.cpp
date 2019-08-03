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

void UMapComponent::OnMapComponentConstruction()
{
	if (IS_VALID(GetOwner()) == false							   // The owner is not valid
		|| !IS_VALID(USingletonLibrary::GetLevelMap(GetWorld())))  // The Level Map is not valid
	{
		return;
	}

	// Find new Location at dragging and update-delegate
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t FCell()");
	Cell = FCell(GetOwner());

	// Owner updating
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t AddActorOnMapByObj");
	USingletonLibrary::GetLevelMap(GetWorld())->AddActorOnMapByObj(Cell, GetOwner());

	// Binds to Owner's OnConstruction to reruncalls the non-generated actors on the Level Map
	if (USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.IsBoundToObject(GetOwner()) == false)
	{
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddUObject(GetOwner(), &AActor::RerunConstructionScripts);
	}

#if WITH_EDITOR  // [PIE]
	if (IS_PIE(GetWorld()) == true)
	{
		// Remove all text renders of the Owner
		USingletonLibrary::PrintToLog(GetOwner(), "[PIE]OnMapComponentConstruction", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(GetOwner());

		// Update AI renders after adding obj to map
		USingletonLibrary::PrintToLog(GetOwner(), "[PIE]OnMapComponentConstruction", "-> \t BroadcastAiUpdating");
		USingletonLibrary::BroadcastAiUpdating();
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
	USingletonLibrary::PrintToLog(GetOwner(), "OnComponentCreated", "Set's defaults");

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
