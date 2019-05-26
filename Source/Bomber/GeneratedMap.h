// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GeneratedMap.generated.h"

UENUM(BlueprintType)
enum class EPathTypesEnum : uint8
{
	Explosion,
	Free,
	Safe,
	Secure
};

UCLASS()
class BOMBER_API AGeneratedMap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeneratedMap();

	// Pathfinding
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "C++")
		void GetSideLocations(FVector vector, int32 sideLength, EPathTypesEnum pathfinder, TSet<FVector>& result);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
