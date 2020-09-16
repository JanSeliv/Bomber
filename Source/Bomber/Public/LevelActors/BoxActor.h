// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "BoxActor.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UBoxDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBoxDataAsset();

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++ | Custom")
    FORCEINLINE int32 GetSpawnItemChance() const { return SpawnItemChanceInternal; }

protected:
	/** The chance to spawn item after box destroying. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++ | Custom", meta = (BlueprintProtected, DisplayName = " Spawn Item Chance", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 SpawnItemChanceInternal = 30.F;  //[D]
};

/**
 * Boxes on destruction with some chances spawns an item.
 */
UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal;	//[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Box Mesh Component"))
	class UStaticMeshComponent* BoxMeshComponentInternal;  //[C.DO]



	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Event triggered when the actor has been explicitly destroyed.
	 * With some chances spawns an item.
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBoxDestroyed(AActor* DestroyedActor);
};
