// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "Components/ActorComponent.h"

#include "MapComponent.generated.h"

/** 
 * These components manage their level actors updates on the level map in case of any changes that allow to:
 * -  Free location and rotation of the level map in the editor time:
 * - Prepare in advance the level actors in the editor time:
 * Same calls and initializations for each of the level map actors
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOMBER_API UMapComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UMapComponent();

	/**
	 * Should be called in the owner's OnConstruction event
	 * Updates a owner's state
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnMapComponentConstruction();

	/** Owner's cell location on the Level Map */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell Cell;

#if WITH_EDITORONLY_DATA  // [Editor]
	/** Mark the editor updating visualization(text renders) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	bool bShouldShowRenders = false;
#endif  //WITH_EDITORONLY_DATA [Editor]

protected:
	/** Called when a component is registered (not loaded) */
	virtual void OnRegister() final;

	/** Called when a component is destroyed */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) final;
};
