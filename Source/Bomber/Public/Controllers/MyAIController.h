﻿// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "AIController.h"
#include "Bomber.h"
#include "Structures/Cell.h"
//---
#include "MyAIController.generated.h"

/**
* Contains AI data.
*/
UCLASS(Blueprintable, BlueprintType)
class UAIDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the AI data asset. */
	static const UAIDataAsset& Get();

	/** Returns the search radius of items.
	  * @see UAIDataAsset::ItemSearchRadiusInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetItemSearchRadius() const { return ItemSearchRadiusInternal; }

	/** Returns the search radius of crossways
	  * @see UAIDataAsset::CrosswaySearchRadiusInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetCrosswaySearchRadius() const { return CrosswaySearchRadiusInternal; }

	/** Returns the filter radius of near cells.
	  * @see UAIDataAsset::NearFilterRadiusInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNearFilterRadius() const { return NearFilterRadiusInternal; }

	/** Returns the radius of dangerous cells.
	* @see UAIDataAsset::DangerousCellRadiusInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNearDangerousRadius() const { return NearDangerousRadiusInternal; }

protected:
	/** The search radius of items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Item Search Radius", ShowOnlyInnerProperties))
	int32 ItemSearchRadiusInternal = 2; //[D]

	/** The search radius of crossways. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Crossway Search Radius", ShowOnlyInnerProperties))
	int32 CrosswaySearchRadiusInternal = 2; //[D]

	/** Determine radius of near dangerous cells (length <= near dangerous radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Near Dangerous Radius", ShowOnlyInnerProperties))
	int32 NearDangerousRadiusInternal = 3; //[D]

	/** Determine filter radius of near cells (length <= near radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Near Filter Radius", ShowOnlyInnerProperties))
	int32 NearFilterRadiusInternal = 3; //[D]
};

/**
 * Characters controlled by bots
 */
UCLASS()
class AMyAIController final : public AAIController
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Sets default values for this character's properties */
	AMyAIController();

	/** Makes AI go toward specified destination cell */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MoveToCell(const struct FCell& DestinationCell);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	friend class UMyCheatManager;

	/** Timer to update AI. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Update Handle"))
	FTimerHandle AIUpdateHandleInternal; //[G]

	/** Cell position of current path segment's end */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "AI Move To"))
	FCell AIMoveToInternal; //[G]

	/** Controlled character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Owner Character"))
	TObjectPtr<class APlayerCharacter> OwnerInternal = nullptr; //[G]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Allows the PlayerController to set up custom input bindings. */
	virtual void OnPossess(APawn* InPawn) override;

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateAI();

	/** Enable or disable AI for this bot. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintProtected))
	void SetAI(bool bShouldEnable) const;

	/** Listen game states to enable or disable AI. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
