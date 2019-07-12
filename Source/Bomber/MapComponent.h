// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Components/ActorComponent.h"

#include "MapComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMapComponent();

	// Callback function to the delegate
	UFUNCTION()
	void UpdateSelfOnMap();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	struct FCell Cell;

protected:
	/** Called when a component is created (not loaded). This can happen in the editor or during gameplay */
	virtual void OnComponentCreated() final;
};
