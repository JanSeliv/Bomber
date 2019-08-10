// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "BoxActor.generated.h"

/**
 * Boxes on destruction with some chances spawns an item.
 */
UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	/** The static mesh component of the this actor */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UStaticMeshComponent* BoxMeshComponent;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** 
	 * Event triggered when the actor has been explicitly destroyed
	 * on destruction with some chances spawns an item
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnBoxDestroyed(AActor* DestroyedActor);
};
