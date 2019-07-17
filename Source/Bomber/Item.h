// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "GameFramework/Actor.h"

#include "Item.generated.h"

UCLASS()
class BOMBER_API AItem final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	UPROPERTY(BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* ItemMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UBoxComponent* ItemCollisionComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	EItemTypeEnum ItemType = EItemTypeEnum::None;

	/** @addtogroup actor_types
	 * Type and its class as associated pairs  */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EItemTypeEnum, class UStaticMesh*> ItemTypesByMeshes;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

#if WITH_EDITOR
	/** Called after an actor has been moved in the editor */
	virtual void PostEditMove(bool bFinished) final;
#endif  //WITH_EDITOR [Editor]

	/**
	 * Called when character starts to overlaps the ItemCollisionComponent component
	 * Increases +1 to skate\fire\bomb amount to the character
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
