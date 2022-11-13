// Copyright (c) Yevhenii Selivanov


#include "SurrounderLogicComponent.h"


// Sets default values for this component's properties
USurrounderLogicComponent::USurrounderLogicComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USurrounderLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USurrounderLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

