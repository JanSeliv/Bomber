// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "WallActor.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UWallDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UWallDataAsset();
};


/**
 * Walls are not destroyed by a bomb explosion and stop the explosion.
 */
UCLASS()
class BOMBER_API AWallActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AWallActor();

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal;	//[C.AW]

	/** The static mesh component of this actor */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Wall Mesh Component"))
	class UStaticMeshComponent* WallMeshComponentInternal;  //[C.DO]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Event triggered when the actor has been explicitly destroyed.
	 * @warning Should not be destroyed in the game
	 */
	UFUNCTION()
	void OnWallDestroyed(AActor* DestroyedActor) { check(!"The wall should never be destroyed"); }
};
