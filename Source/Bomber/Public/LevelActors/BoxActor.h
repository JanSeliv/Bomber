// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Globals/LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "BoxActor.generated.h"

/**
 * Describes common data for all boxes.
 */
UCLASS(Blueprintable, BlueprintType)
class UBoxDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBoxDataAsset();

	/** Returns the box data asset. */
	static const UBoxDataAsset& Get();

	/** Returns the chance to spawn item after box destroying. */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int32 GetSpawnItemChance() const { return SpawnItemChanceInternal; }

protected:
	/** The chance to spawn item after box destroying. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Spawn Item Chance", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 SpawnItemChanceInternal = 30.F; //[D]
};

/**
 * Boxes on destruction with some chances spawns an item.
 */
UCLASS()
class ABoxActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABoxActor();

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal; //[C.AW]

	/** Contains current spawn chance to spawn item. Can be overriden by the Cheat Manager. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Spawn Item Chance"))
	int32 SpawnItemChanceInternal = INDEX_NONE; //[G]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Spawn item with a chance. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (BlueprintProtected))
	void TrySpawnItem(AActor* DestroyedActor = nullptr);

	/** The item chance can be overrided in game, so it should be reset for each new game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateItemChance();

	/** Listen to reset item chance for each new game. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
