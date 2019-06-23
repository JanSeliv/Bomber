// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class BOMBER_API AItem final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) final;

	UFUNCTION()
	void OnItemBeginOverlap(AActor* OverlappedItem, AActor* OtherActor);
};
