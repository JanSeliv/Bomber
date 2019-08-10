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
	if (IS_VALID(GetOwner()) == false					 // The owner is not valid
		|| !IS_VALID(USingletonLibrary::GetLevelMap()))  // The Level Map is not valid
	{
		return;
	}

	// Find new Location at dragging and update-delegate
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t FCell()");
	Cell = FCell(GetOwner());

	// Owner updating
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t AddActorToGridArray");
	USingletonLibrary::GetLevelMap()->AddActorToGridArray(Cell, GetOwner());

#if WITH_EDITOR  // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Remove all text renders of the Owner
		USingletonLibrary::PrintToLog(GetOwner(), "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(GetOwner());

		// Update AI renders after adding obj to map
		USingletonLibrary::PrintToLog(GetOwner(), "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t BroadcastAiUpdating");
		USingletonLibrary::BroadcastAiUpdating();
	}
#endif  //WITH_EDITOR [IsEditorNotPieWorld]
}

void UMapComponent::OnRegister()
{
	Super::OnRegister();
	if (IS_VALID(GetOwner()) == false)  // owner is not valid
	{
		return;
	}
	USingletonLibrary::PrintToLog(GetOwner(), "OnRegister", "");

	// Disable the tick
	GetOwner()->SetActorTickEnabled(false);

	// Set the movable mobility for in-game attaching
	if (GetOwner()->GetRootComponent() != nullptr)
	{
		GetOwner()->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

#if WITH_EDITOR									   // [Editor]
	if (USingletonLibrary::IsEditorNotPieWorld())  // PIE only
	{
		// Should not call OnConstruction on drag events
		GetOwner()->bRunConstructionScriptOnDrag = false;
	}

	// Binds to Owner's OnConstruction to rerun calls the non-generated actors on the Level Map
	// don't call OnActorsUpdatedDelegate in gameplay
	if (GetOwner() != nullptr)
	{
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddUObject(GetOwner(), &AActor::RerunConstructionScripts);
	}
#endif  //WITH_EDITOR [Editor]
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IS_TRANSIENT(GetOwner()) == false				// Is not transient owner
		&& IS_VALID(USingletonLibrary::GetLevelMap()))  // is valid and is not transient the level map
	{
		USingletonLibrary::PrintToLog(GetOwner(), "OnComponentDestroyed", "-> \t RemoveActorFromGridArray");
		USingletonLibrary::GetLevelMap()->RemoveActorFromGridArray(GetOwner());

#if WITH_EDITOR  // [IsEditorNotPieWorld]
		if (USingletonLibrary::IsEditorNotPieWorld())
		{
			USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.RemoveAll(GetOwner());
			USingletonLibrary::BroadcastAiUpdating();
		}
#endif  //WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
