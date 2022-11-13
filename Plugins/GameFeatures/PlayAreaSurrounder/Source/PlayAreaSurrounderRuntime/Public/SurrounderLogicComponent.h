// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/ActorComponent.h"
#include "SurrounderLogicComponent.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLAYAREASURROUNDERRUNTIME_API USurrounderLogicComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USurrounderLogicComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
