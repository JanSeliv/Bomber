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
	CHECKMAP();
	cellLocation = FCell(GetOwner());
	USingletonLibrary::GetLevelMap()->AddActorOnMapByObj(GetOwner());
}

void UMapComponent::OnRegister()
{
	Super::OnRegister();

	CHECKMAP();
	if (IsValid(GetOwner()) == false) return;

	USingletonLibrary::GetLevelMap()->onActorsUpdateDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);
	UpdateSelfOnMap();
}

void UMapComponent::BeginDestroy()
{
	Super::BeginDestroy();
	CHECKMAP();
	USingletonLibrary::GetLevelMap()->DestroyActorFromMap(cellLocation);
}
