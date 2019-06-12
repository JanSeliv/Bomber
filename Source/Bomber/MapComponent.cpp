// Fill out your copyright notice in the Description page of Project Settings.


#include "MapComponent.h"
#include "Bomber.h"

// Sets default values for this component's properties
UMapComponent::UMapComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UMapComponent::UpdateSelfOnMap()
{
	if (!ISVALID(owner) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT(owner)) return;

	if (owner->IsA(ACharacter::StaticClass()))
	{
		cell = FCell(owner); // !!! Update character location again
		UE_LOG_STR("UpdateSelfOnMap: %s: Character cell updating", *owner->GetName());
	}
	USingletonLibrary::GetLevelMap()->AddActorOnMapByObj(cell, owner);
	UE_LOG_STR("UpdateSelfOnMap: %s UPDATED", *owner->GetName());
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	owner = GetOwner();
	if (!ISVALID(owner) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT(owner)) return;

	cell = FCell(owner);

	// Shouldt call OnConstruction on drag events
	owner->bRunConstructionScriptOnDrag = false;

	// Push owner to regenerated TMap
	USingletonLibrary::GetLevelMap()->onActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);

	UE_LOG_STR("OnComponentCreated: %s", *owner->GetName());
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (owner != nullptr && ISVALID(USingletonLibrary::GetLevelMap()) && !ISTRANSIENT(owner))
	{
		UE_LOG_STR("OnComponentDestroyed %s:", *owner->GetName());
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}




