// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cell.h"
#include "GameFramework/Actor.h"

#include "Bomb.generated.h"

UCLASS()
class BOMBER_API ABomb final : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABomb();

	/**
	 * Sets the defaults of the bomb
	 * @param RefBombsN Reference to the character's bombs count to change an amount after bomb putting(--) and destroying(++)
	 * @param FireN Setting explosion length of this bomb
	 * @param CharacterID Setting a mesh material of bomb by the character ID
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitializeBombProperties(
		UPARAM(ref) int32& RefBombsN,
		const int32& FireN,
		const int32& CharacterID);

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UStaticMeshComponent* BombMeshComponent;

	/** Prevents players from moving through the bomb after they moved away */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UBoxComponent* BombCollisionComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class UParticleSystem* ExplosionParticle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() final;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) final;

	/** 
	 * Event triggered when the actor has been explicitly destroyed
	 * Destroys all actors from array of cells
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnBombDestroyed(AActor* DestroyedActor);

	/**
	 * Called when character end to overlaps the BombCollisionComponent component
	 * Sets the collision preset to block all dynamics
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnBombEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, Category = "C++")
	float LifeSpan_ = 2.f;

	UPROPERTY(EditAnywhere, Category = "C++")
	int32 ExplosionLength = 1;

	// Amount of character bombs at current time
	int32* CharacterBombsN_;

	// All used bomb materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	TArray<class UMaterialInterface*> BombMaterials_;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	TSet<FCell> ExplosionCells_;
};
