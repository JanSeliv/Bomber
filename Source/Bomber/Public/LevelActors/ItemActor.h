﻿// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "ItemActor.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UItemRow final : public ULevelActorRow
{
	GENERATED_BODY()

public:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	EItemType ItemType = EItemType::None; //[D]
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UItemDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UItemDataAsset();
};

/**
 * Affects the abilities of a player during gameplay
 */
UCLASS()
class BOMBER_API AItemActor final : public AActor
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	AItemActor();

	/** Return current item type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE EItemType GetItemType() const { return ItemTypeInternal; }

	/** Calls to uninitialize item type. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	FORCEINLINE EItemType ResetItemType() { return ItemTypeInternal = EItemType::None; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal; //[C.AW]

	/**
	* Skate: Increase the movement speed of the character.
	* Bomb: Increase the number of bombs that can be set at one time.
	* Fire: Increase the bomb blast radius.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DIsplayName = "Item Type"))
	EItemType ItemTypeInternal = EItemType::None; // [AW]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Triggers when this item starts overlap a player character to destroy itself. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
