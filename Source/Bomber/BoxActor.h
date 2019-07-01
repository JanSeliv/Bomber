// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "BoxActor.generated.h"

UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

	/** The Map Component manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** 
	 * Event triggered when the actor has been explicitly destroyed
	 * Spawn item with the chance 
	 */
	UFUNCTION()
	void OnBoxDestroyed(AActor* DestroyedActor);
};
