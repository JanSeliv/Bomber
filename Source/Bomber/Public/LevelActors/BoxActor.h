// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "BoxActor.generated.h"

/**
 * Boxes on destruction with some chances spawns an item.
 */
UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;	//[C.AW]

	/** The static mesh component of the this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* BoxMeshComponent;  //[C.DO]

	/** Sets default values for this actor's properties */
	ABoxActor();

protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** 
	 * Event triggered when the actor has been explicitly destroyed.
	 * With some chances spawns an item.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBoxDestroyed(AActor* DestroyedActor);
};
