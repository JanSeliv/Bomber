// Copyright (c) Yevhenii Selivanov

#include "ProgressionSystemComponent.h"
//---
#include "ProgressionSystemRuntimeModule.h"

// Sets default values for this component's properties
UProgressionSystemComponent::UProgressionSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when the game starts
void UProgressionSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogProgressionSystem, Warning, TEXT("--- I'm running ---"));
}
