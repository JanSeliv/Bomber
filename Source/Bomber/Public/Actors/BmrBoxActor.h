// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"

#include "BmrBoxActor.generated.h"

/**
 * Boxes on destruction with some chances spawns an powerup.
 * @see Access its data with UBmrBoxDataAsset (Content/Bomber/DataAssets/DA_Box).
 */
UCLASS()
class BOMBER_API ABmrBoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABmrBoxActor();

	/** Spawn powerup with a chance. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void TrySpawnPowerup();

protected:
	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMapComponent> MapComponent = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnAddedToLevel(UBmrMapComponent* InMapComponent);

	/** Called when this level actor is destroyed on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(UBmrMapComponent* InMapComponent, UObject* DestroyCauser);
};