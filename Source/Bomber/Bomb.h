// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Bomb.generated.h"

UCLASS()
class BOMBER_API ABomb : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABomb();

	UPROPERTY(BlueprintReadOnly, Category = "C++")
	class UMapComponent* mapComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when this actor is explicitly being destroyed
	virtual void Destroyed() override;

	/** 
	 *	Event when an actor no longer overlaps another actor, and they have separated. 
	 *	@note Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
	 */
	virtual void NotifyActorEndOverlap(AActor* OtherActor);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	float lifeSpan = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 explosionLen = 1;

	struct FPowerUp* characterPowerUps;

};
