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
	if (!ISVALID(GetOwner()) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT) return;

	cellLocation = FCell(GetOwner());
	USingletonLibrary::GetLevelMap()->AddActorOnMapByObj(GetOwner());
	PRINT(GetOwner()->GetName() + " UPDATED");
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	if (!ISVALID(GetOwner()) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT) return;

	// Component instance will be dynamically created 
	//CreationMethod = EComponentCreationMethod::UserConstructionScript;

	// Shouldt call OnConsturction on drag events
	GetOwner()->bRunConstructionScriptOnDrag = false;

	// Push owner to regenerated TMap
	USingletonLibrary::GetLevelMap()->onActorsUpdateDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);

	PRINT("Component created");
}

void UMapComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	if (!ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT) return;

	USingletonLibrary::GetLevelMap()->DestroyActorFromMap(cellLocation);
	PRINT("Component destroyed");
}
