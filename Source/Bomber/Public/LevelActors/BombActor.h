// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Cell.h"
#include "GameFramework/Actor.h"

#include "BombActor.generated.h"

/** Bombs are left by the character to destroy the level actors, trigger other bombs */
UCLASS()
class BOMBER_API ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;  //[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* BombMeshComponent;  //[C.DO]

	/** All materials that used by bomb meshes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TArray<class UMaterialInterface*> BombMaterials;  //[M.DO]

	/** Prevents players from moving through the bomb after they moved away */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UBoxComponent* BombCollisionComponent;  //[C.DO]

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	class UParticleSystem* ExplosionParticle;  //[B]

	/** Sets default values for this actor's properties */
	ABombActor();

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

protected:
	/** The lifetime of a bomb*/
	UPROPERTY(EditAnywhere, Category = "C++", meta = (BlueprintProtected))
	float LifeSpan_ = 2.f;

	/** The blast length on each side */
	UPROPERTY(EditAnywhere, Category = "C++", meta = (BlueprintProtected))
	int32 ExplosionLength_ = 1;

	/** Amount of character bombs at the current time */
	int32* PlayerBombsN_;

	/** The bomb blast path */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++", meta = (BlueprintProtected))
	TSet<struct FCell> ExplosionCells_;
	/** The level map has access to ABombActor::ExplosionCells_ */
	friend class AGeneratedMap;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** 
	 * Event triggered when the actor has been explicitly destroyed.
	 * Calls destroying request of all actors by cells in explosion cells array.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombDestroyed(AActor* DestroyedActor);

	/**
	 * Called when character end to overlaps the BombCollisionComponent component.
	 * Sets the collision preset to block all dynamics.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
