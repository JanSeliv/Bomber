// Fill out your copyright notice in the Description page of Project Settings.

#include "MapComponent.h"

#include "Bomber.h"
#include "BoxActor.h"
#include "GeneratedMap.h"
#include "MyCharacter.h"
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

	USingletonLibrary::GetLevelMap(World)->AddActorOnMapByObj(Cell, GetOwner());

// Update AI renders after adding obj to map
#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true						  // for editor only
		&& USingletonLibrary::GetSingleton() != nullptr)  // Singleton is not null
	{
		UE_LOG_STR("PIE:UpdateSelfOnMap: %s BROADCAST AI updating", GetOwner());
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
	}
#endif  //WITH_EDITOR
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (IS_VALID(GetOwner()) == false)  // owner is not valid
	{
		return;
	}
	UE_LOG_STR("OnComponentCreated: %s", GetOwner());

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
	}
#endif  //WITH_EDITOR
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
// Update AI renders after destroying obj from map
#if WITH_EDITOR
	if (IS_PIE(GetWorld()) == true		 // for editor only
		&& IS_TRANSIENT(this) == false)  // Component is not transient
	{
		UE_LOG_STR("PIE:OnComponentDestroyed: %s BROADCAST AI updating", GetOwner());
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
	}
#endif  //WITH_EDITOR
}