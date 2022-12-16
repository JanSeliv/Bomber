// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "GameFramework/Actor.h"
//---
#include "ItemActor.generated.h"

/**
 * Row that describes each unique item.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UItemRow final : public ULevelActorRow
{
	GENERATED_BODY()

public:
	/** Of each type this item is. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	EItemType ItemType = EItemType::None;
};

/**
 * Describes common data for all items.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UItemDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UItemDataAsset();

	/** Returns the item data asset. */
	static const UItemDataAsset& Get();

	/** Returns speed value that is added to the player speed on taking a skate item.
	  * @see UItemDataAsset::SkateStrengthInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetSkateAdditiveStrength() const { return SkateAdditiveStrengthInternal; }

	/** Return row by specified item type. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const UItemRow* GetRowByItemType(EItemType ItemType, ELevelType LevelType) const;

	/** Returns max possible items to be picked up by player.
	  * @see UItemDataAsset::MaxAllowedItemNumInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMaxAllowedItemsNum() const { return MaxAllowedItemsNumInternal; }

protected:
	/** The speed additive value when player takes the skate item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skate Additive Strength", ShowOnlyInnerProperties))
	float SkateAdditiveStrengthInternal = 500.f;

	/** Max possible items to be picked up by player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Max Allowed Items Num", ShowOnlyInnerProperties))
	int32 MaxAllowedItemsNumInternal = 5;
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

	/** The MapComponent manages this actor on the Level Map */
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
