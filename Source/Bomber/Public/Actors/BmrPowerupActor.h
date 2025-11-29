// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "ItemActor.generated.h"

/**
 * Affects the abilities of a player during gameplay.
 * @see Access its data with UItemDataAsset (Content/Bomber/DataAssets/DA_Item).
 */
UCLASS()
class BOMBER_API AItemActor final : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AItemActor();

	/** Return current item type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE FBmrPowerupTag GetItemType() const { return ItemTypeInternal; }

	/** Set new item type, can be called on the server-only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetItemType(FBmrPowerupTag NewItemType);

protected:
	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/**
	 * Skate: Increase the movement speed of the character.
	 * Bomb: Increase the number of bombs that can be set at one time.
	 * Fire: Increase the bomb blast radius.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_ItemType", Category = "C++", meta = (BlueprintProtected, DisplayName = "Item Type"))
	FBmrPowerupTag ItemTypeInternal = FBmrPowerupTag::None;

	/** Is called on client when item type is replicated. */
	UFUNCTION()
	void OnRep_ItemType();

	/** Is called on both server and clients to update the item mesh based on the item type. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateItemMesh();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAddedToLevel(UMapComponent* MapComponent);

	/** Triggers when this item starts overlap a player character to destroy itself. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Called when this level actor is destroyed from the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(UMapComponent* MapComponent, UObject* DestroyCauser);
};