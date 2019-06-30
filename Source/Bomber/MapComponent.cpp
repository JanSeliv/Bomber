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
	UWorld* const World = GetWorld();
	if (World == nullptr									  // World is null
		|| IS_VALID(GetOwner()) == false					  // owner is not valid
		|| USingletonLibrary::GetLevelMap(World) == nullptr)  // levelMap is null
	{
		return;
	}

	Cell = FCell(GetOwner());  // Find new Location at dragging and update-delegate

	USingletonLibrary::GetLevelMap(World)->AddActorOnMapByObj(Cell, GetOwner());

// Update AI renders after adding obj to map
#if WITH_EDITOR
	if (World->HasBegunPlay() == false)  // for editor only
	{
		USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
		UE_LOG_STR("PIE:UpdateSelfOnMap: %s BROADCAST AI updating", GetOwner());
	}
#endif  //WITH_EDITOR

	UE_LOG(LogTemp, Warning, TEXT("UpdateSelfOnMap: %s UPDATED (%s)"), *GetOwner()->GetName(), *Cell.Location.ToString());
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

// Binds to updating actors on the Level Map
#if WITH_EDITOR
	if (GetWorld() != nullptr							  // World is not null
		&& GetWorld()->HasBegunPlay() == false			  // for editor only
		&& USingletonLibrary::GetSingleton() != nullptr)  // Singleton is valid
	{
		USingletonLibrary::GetSingleton()->OnActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);
	}
#endif  //WITH_EDITOR

	UE_LOG_STR("OnComponentCreated: %s", GetOwner());
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	UWorld* const World = GetWorld();
	if (World != nullptr									 // World is not null
		&& USingletonLibrary::GetLevelMap(World) != nullptr  // LevelMap_ is valid
		&& IS_TRANSIENT(this) == false)						 // Component is not transient
	{
		const ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character != nullptr &&
			USingletonLibrary::GetLevelMap(World)->CharactersOnMap_.Contains(Character))
		{
			USingletonLibrary::GetLevelMap(World)->CharactersOnMap_.Remove(Character);
			UE_LOG_STR("OnComponentDestroyed: %s removed from TSet", GetOwner());
		}

// Update AI renders after destroying obj from map
#if WITH_EDITOR
		if (World->HasBegunPlay() == false)  // for editor only
		{
			USingletonLibrary::GetSingleton()->OnRenderAiUpdatedDelegate.Broadcast();
			UE_LOG_STR("PIE:UpdateSelfOnMap: %s BROADCAST AI updating", GetOwner());
		}
#endif  //WITH_EDITOR

		UE_LOG_STR("OnComponentDestroyed: %s was destroyed", GetOwner());
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
