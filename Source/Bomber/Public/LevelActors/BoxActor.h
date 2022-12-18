// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "BoxActor.generated.h"

/**
 * Boxes on destruction with some chances spawns an item.
 * @see Access its data with UBoxDataAsset (Content/Bomber/Globals/DA_Box).
 */
UCLASS()
class BOMBER_API ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

	/** Initialize a box actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructBoxActor();

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/** Contains current spawn chance to spawn item. Can be overriden by the Cheat Manager. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Spawn Item Chance"))
	int32 SpawnItemChanceInternal = INDEX_NONE;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on a box actor construction, could be called multiple times.
	 * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	* AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructBoxActor() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionBoxActor().
	 * @warning Do not call directly, use ThisClass::ConstructBoxActor() instead. */
	UFUNCTION()
	void OnConstructionBoxActor();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Called when owned map component is destroyed on the level map. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnDeactivatedMapComponent(UMapComponent* MapComponent, UObject* DestroyCauser);

	/** Spawn item with a chance. */
	UFUNCTION(BlueprintPure = false, Category = "C++", meta = (BlueprintProtected))
	void TrySpawnItem();

	/** The item chance can be overrided in game, so it should be reset for each new game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateItemChance();

	/** Listen to reset item chance for each new game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
