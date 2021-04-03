// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Structures/Cell.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "BombActor.generated.h"

/**
 * Describes common data for all bombs.
 */
UCLASS(Blueprintable, BlueprintType)
class UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBombDataAsset();

	/** Returns the bomb data asset. */
	static const UBombDataAsset& Get();

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TArray<class UMaterialInterface*> BombMaterials; //[D]

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	class UParticleSystem* ExplosionParticle; //[D]

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (ShowOnlyInnerProperties))
	FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Life Span", ShowOnlyInnerProperties))
	float LifeSpanInternal = 2.f; //[D]
};

/** Bombs are left by the character to destroy the level actors, trigger other bombs */
UCLASS()
class BOMBER_API ABombActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Is used by player character to listen bomb destroying. */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBombDestroyed, AActor*, DestroyedBomb);

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	ABombActor();

	/** Returns explosion cells (by copy to avoid changes). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSet<FCell> GetExplosionCells() const { return ExplosionCellsInternal; }

	/**
	 * Sets the defaults of the bomb
	 * @param EventToBind Delegate that will be executed on bomb destroying
	 * @param FireN Setting explosion length of this bomb
	 * @param CharacterID Setting a mesh material of bomb by the character ID
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void InitBomb(
		const FOnBombDestroyed& EventToBind,
		int32 FireN = 1,
		int32 CharacterID = -1);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal; //[C.AW]

	/** The bomb blast path */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++", meta = (BlueprintProtected, DisplayName = "Explosion Cells", ShowOnlyInnerProperties))
	TSet<FCell> ExplosionCellsInternal;

	/* ---------------------------------------------------
 	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Set the lifespan of this actor. When it expires the object will be destroyed.
	 * @param InLifespan overriden with a default value, time will be got from the data asset. */
	virtual void SetLifeSpan(float InLifespan = INDEX_NONE) override;

	/** Called when the lifespan of an actor expires (if he has one). */
	virtual void LifeSpanExpired() override;

	/** Destroy bomb and burst explosion cells.
	  * Calls destroying request of all actors by cells in explosion cells array.*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DetonateBomb(AActor* DestroyedActor = nullptr);

	/**
	 * Triggers when character end to overlaps with this bomb.
	 * Sets the collision preset to block all dynamics.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Listen by dragged bombs to handle game resetting. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
