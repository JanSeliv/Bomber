// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Components/ActorComponent.h"

#include "MapComponent.generated.h"

/** Same calls and initializations for each of the level map actors */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMapComponent();

	/**
	 * Should be called in the owner's OnConstruction event
	 * Updates a owner's state */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnMapComponentConstruction();

	/** Owner's cell location on the Level Map */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell Cell;

protected:
	/**
	 * Called when a component is created (not loaded)
	 * Sets owner's defaults
	 */
	virtual void OnComponentCreated() final;

	/* Called when a component is destroyed */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
};
