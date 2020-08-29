// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Cell.h"
#include "LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "BombActor.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBombDestroyed, AActor*, DestroyedBomb);

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
    FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

protected:
	/** The lifetime of a bomb*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++")
	float LifeSpanInternal = 2.f;
};

/** Bombs are left by the character to destroy the level actors, trigger other bombs */
UCLASS()
class BOMBER_API ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;	//[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* BombMeshComponent;	//[C.DO]

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
	 * @param EventToBind Delegate that will be executed on bomb destroying
	 * @param FireN Setting explosion length of this bomb
	 * @param CharacterID Setting a mesh material of bomb by the character ID
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitBomb(
		const FOnBombDestroyed& EventToBind,
		const int32& FireN = 1,
		const int32& CharacterID = -1);

protected:
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
