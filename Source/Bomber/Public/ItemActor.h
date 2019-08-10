// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bomber.h"
#include "GameFramework/Actor.h"

#include "ItemActor.generated.h"

/** 
 * Affects the abilities of a player during gameplay
 */
UCLASS()
class BOMBER_API AItemActor final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItemActor();

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	/** The static mesh component of the this actor */
	UPROPERTY(BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* ItemMeshComponent;

	/** Type and its class as associated pairs  */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	TMap<EItemTypeEnum, class UStaticMesh*> ItemTypesByMeshes;

	/**
	 * Skate: Increase the movement speed of the character.
	 * Bomb: Increase the number of bombs that can be set at one time.
	 * Fire: Increase the bomb blast radius.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "C++")
	EItemTypeEnum ItemType = EItemTypeEnum::None;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/**
	 * Called when character starts to overlaps the ItemCollisionComponent component
	 * Increases +1 to numbers of character's powerups (Skate/Bomb/Fire)
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
