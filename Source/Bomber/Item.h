// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "Item.generated.h"

/*
 * @todo Add the EItemTypeEnum
 */
UCLASS()
class BOMBER_API AItem final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AItem();

	/** The Map Component manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/**
	 * Called when another actor begins to overlap this actor
	 * @todo Finish the code OnItemBeginOverlap(...)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void OnItemBeginOverlap(AActor* OverlappedItem, AActor* OtherActor);
};
