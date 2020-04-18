// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "WallActor.generated.h"

/**
 * Walls are not destroyed by a bomb explosion and stop the explosion.
 */
UCLASS()
class BOMBER_API AWallActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AWallActor();

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;	//[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* WallMeshComponent;	//[C.DO]

protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** 
	 * Event triggered when the actor has been explicitly destroyed.
	 * @warning Should not be destroyed in the game
	 */
	UFUNCTION()
	void OnWallDestroyed(AActor* DestroyedActor) { check(!"The wall should never be destroyed"); }
};
