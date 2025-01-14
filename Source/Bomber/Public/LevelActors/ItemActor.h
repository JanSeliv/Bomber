// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Actor.h"
//---
#include "Bomber.h"
//---
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
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Sets default values for this actor's properties */
	AItemActor();

	/** Initialize an item actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructItemActor();

	/** Return current item type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE EItemType GetItemType() const { return ItemTypeInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/**
	* Skate: Increase the movement speed of the character.
	* Bomb: Increase the number of bombs that can be set at one time.
	* Fire: Increase the bomb blast radius.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "C++", meta = (BlueprintProtected, DIsplayName = "Item Type"))
	EItemType ItemTypeInternal = EItemType::None;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on an item actor construction, could be called multiple times.
	 * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructItemActor() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionItemActor().
	 * @warning Do not call directly, use ThisClass::ConstructItemActor() instead. */
	UFUNCTION()
	void OnConstructionItemActor();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Triggers when this item starts overlap a player character to destroy itself. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Calls to uninitialize item type. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	EItemType ResetItemType() { return ItemTypeInternal = EItemType::None; }
};