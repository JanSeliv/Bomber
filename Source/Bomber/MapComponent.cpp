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
	if (ISVALID(GetOwner()) == false							   // owner is not valid
		|| USingletonLibrary::GetLevelMap(GetWorld()) == nullptr)  // levelMap is null
	{
		return;
	}

	cell = FCell(GetOwner());  // Find new location at dragging and update-delegate

	USingletonLibrary::GetLevelMap(GetWorld())->AddActorOnMapByObj(cell, GetOwner());

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

	if (ISVALID(GetOwner()) == false							   // owner is not valid
		|| USingletonLibrary::GetLevelMap(GetWorld()) == nullptr)  // levelMap is null
	{
		return;
	}

	// Shouldt call OnConstruction on drag events
	GetOwner()->bRunConstructionScriptOnDrag = false;

	// Push owner to regenerated TMap
	USingletonLibrary::GetLevelMap(GetWorld())->onActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);

	UE_LOG_STR("OnComponentCreated: %s", GetOwner());
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (USingletonLibrary::GetLevelMap(GetWorld()) != nullptr)
	{
		const ACharacter* character = Cast<ACharacter>(GetOwner());
		if (character != nullptr &&
			USingletonLibrary::GetLevelMap(GetWorld())->charactersOnMap_.Contains(character))
		{
			USingletonLibrary::GetLevelMap(GetWorld())->charactersOnMap_.Remove(character);
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
