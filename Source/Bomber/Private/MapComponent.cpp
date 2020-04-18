// Copyright 2020 Yevhenii Selivanov.

#include "MapComponent.h"
//---
#include "Bomber.h"
#include "GeneratedMap.h"
#include "SingletonLibrary.h"

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Updates a owner's state. Should be called in the owner's OnConstruction event.
void UMapComponent::OnMapComponentConstruction()
{
	AGeneratedMap* LevelMap = USingletonLibrary::GetLevelMap();
	if (IS_VALID(GetOwner()) == false  // The owner is not valid
		|| !IsValid(LevelMap))		   // The Level Map is not valid
	{
		return;
	}

	// Find new Location at dragging and update-delegate
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t FCell()");
	LevelMap->SetNearestCell(this);

	// Owner updating
	USingletonLibrary::PrintToLog(GetOwner(), "OnMapComponentConstruction", "-> \t AddToGrid");
	USingletonLibrary::GetLevelMap()->AddToGrid(GetCell(), this);

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Remove all text renders of the Owner
		USingletonLibrary::PrintToLog(GetOwner(), "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t ClearOwnerTextRenders");
		USingletonLibrary::ClearOwnerTextRenders(GetOwner());

		// Update AI renders after adding obj to map
		USingletonLibrary::PrintToLog(GetOwner(), "[IsEditorNotPieWorld]OnMapComponentConstruction", "-> \t BroadcastAiUpdating");
		USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

//  Called when a component is registered (not loaded
void UMapComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	if (!IS_VALID(Owner))  // owner is not valid
	{
		return;
	}
	USingletonLibrary::PrintToLog(Owner, "OnRegister", "");

	// Finding the actor type
	ActorType = USingletonLibrary::GetActorTypeByClass(Owner->GetClass());
	check(ActorType != EActorType::None && "Is not valid the specified class");

	// Disable the tick
	Owner->SetActorTickEnabled(false);

	// Set the movable mobility for in-game attaching
	if (Owner->GetRootComponent() != nullptr)
	{
		Owner->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Should not call OnConstruction on drag events
		Owner->bRunConstructionScriptOnDrag = false;
	}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
}

// Called when a component is destroyed for removing the owner from the Level Map.
void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	AActor* const ComponentOwner = GetOwner();
	if (IS_TRANSIENT(ComponentOwner) == false		   // Is not transient owner
		&& IsValid(USingletonLibrary::GetLevelMap()))  // is valid and is not transient the level map
	{
		USingletonLibrary::PrintToLog(ComponentOwner, "OnComponentDestroyed", "-> \t DestroyActorsFromMap");
		USingletonLibrary::GetLevelMap()->RemoveMapComponent(this);	 // During a game: destroyed bombs, pickup-ed items

#if WITH_EDITOR	 // [IsEditorNotPieWorld]
		// Editor delegates
		if (USingletonLibrary::IsEditorNotPieWorld())  // [IsEditorNotPieWorld]
		{
			USingletonLibrary::GOnAIUpdatedDelegate.Broadcast();
		}
#endif	//WITH_EDITOR [IsEditorNotPieWorld]
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
