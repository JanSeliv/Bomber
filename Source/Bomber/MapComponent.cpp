// Fill out your copyright notice in the Description page of Project Settings.

#include "MapComponent.h"

#include "Bomber.h"
#include "GeneratedMap.h"
#include "MyCharacter.h"
#include "SingletonLibrary.h"

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UMapComponent::UpdateSelfOnMap()
{
	if (IS_VALID(GetOwner()) == false							   // owner is not valid
		|| USingletonLibrary::GetLevelMap(GetWorld()) == nullptr)  // levelMap is null
	{
		return;
	}

	Cell = FCell(GetOwner());  // Find new Location at dragging and update-delegate

	USingletonLibrary::GetLevelMap(GetWorld())->AddActorOnMapByObj(Cell, GetOwner());

// Update renders after adding obj on map
#if WITH_EDITOR
	if (GetWorld()->HasBegunPlay() == false)  // for editor only
	{
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
		UE_LOG_STR("PIE:UpdateSelfOnMap: %s BROADCAST AI updating", GetOwner());
	}
#endif

	UE_LOG_STR("UpdateSelfOnMap: %s UPDATED", GetOwner());
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (IS_VALID(GetOwner()) == false)  // owner is not valid
	{
		return;
	}

	// Should not call OnConstruction on drag events
	GetOwner()->bRunConstructionScriptOnDrag = false;

// Binds to updating on the Level Map
#if WITH_EDITOR
	if (GetWorld()->HasBegunPlay() == false				  // for editor only
		&& USingletonLibrary::GetSingleton() != nullptr)  // Singleton is valid
	{
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);
	}
#endif

	UE_LOG_STR("OnComponentCreated: %s", GetOwner());
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (USingletonLibrary::GetLevelMap(GetWorld()) != nullptr)
	{
		const ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character != nullptr &&
			USingletonLibrary::GetLevelMap(GetWorld())->CharactersOnMap_.Contains(Character))
		{
			USingletonLibrary::GetLevelMap(GetWorld())->CharactersOnMap_.Remove(Character);
			UE_LOG_STR("OnComponentDestroyed: %s removed from TSet", GetOwner());
		}

// Update AI renders after destroying obj from map
#if WITH_EDITOR
		if (GetWorld()->HasBegunPlay() == false)  // for editor only
		{
			USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
			UE_LOG_STR("PIE:UpdateSelfOnMap: %s BROADCAST AI updating", GetOwner());
		}
#endif

		UE_LOG_STR("OnComponentDestroyed: %s was destroyed", GetOwner());
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
