// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"

#include "BmrWallActor.generated.h"

/**
 * Walls are not destroyed by a bomb explosion and break the explosion.
 * @see Access its data with UBmrWallDataAsset (Content/Bomber/DataAssets/DA_Wall).
 */
UCLASS()
class BOMBER_API ABmrWallActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABmrWallActor();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMapComponent> MapComponent = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnAddedToLevel(UBmrMapComponent* InMapComponent);
};
