// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Cell.h"
#include "LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "BombActor.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UBombDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBombDataAsset();

	/** All bomb materials. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++  | Custom")
	TArray<class UMaterialInterface*> BombMaterials; //[D]

	/** The emitter of the bomb explosion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Custom")
	class UParticleSystem* ExplosionParticle; //[D]

	/** Get the bomb lifetime. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++  | Custom")
    FORCEINLINE float GetLifeSpan() const { return LifeSpanInternal; }

protected:
	/** The lifetime of a bomb. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Custom", meta = (DisplayName = "Life Span"))
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
		const int32& FireN = 1,
		const int32& CharacterID = -1);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal;	//[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Bomb Mesh Component"))
	class UStaticMeshComponent* BombMeshComponentInternal;	//[C.DO]

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

	/**
	 * Event triggered when the actor has been explicitly destroyed.
	 * Calls destroying request of all actors by cells in explosion cells array.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombDestroyed(AActor* DestroyedActor);

	/**
	 * Triggers when character end to overlaps with this bomb.
	 * Sets the collision preset to block all dynamics.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
