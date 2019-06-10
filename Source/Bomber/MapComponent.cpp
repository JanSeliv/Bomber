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

	if (GetOwner()->IsA(ACharacter::StaticClass()))
	{
		cell = FCell(GetOwner()); // !!! Update character location again
		UE_LOG_STR("%s: Character cell updating", *GetOwner()->GetName());
	}
	USingletonLibrary::GetLevelMap()->AddActorOnMapByObj(cell, GetOwner());
	UE_LOG_STR("%s UPDATED", *GetOwner()->GetName());
}

void UMapComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	if (!ISVALID(GetOwner()) || !ISVALID(USingletonLibrary::GetLevelMap()) || ISTRANSIENT) return;

	cell = FCell(GetOwner());

	// Shouldt call OnConsturction on drag events
	GetOwner()->bRunConstructionScriptOnDrag = false;

	// Push owner to regenerated TMap
	USingletonLibrary::GetLevelMap()->onActorsUpdatedDelegate.AddDynamic(this, &UMapComponent::UpdateSelfOnMap);

	UE_LOG_STR("%s", "OnComponentCreated");
}




