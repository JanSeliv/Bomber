// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "AIController.h"

// Bomber
#include "Structures/Cell.h"

#include "MyAIController.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Characters controlled by bots.
 * @see Access its data with UAIDataAsset (Content/Bomber/DataAssets/DA_AI).
 */
UCLASS()
class BOMBER_API AMyAIController final : public AAIController
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
	void MoveToCell(const FCell& DestinationCell);

	/** Returns true if AI is enabled (move input is not ignored and cheat is not enabled). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Is AI Enabled"))
	bool IsAIEnabled() const;

	/** Returns true if AI can spawn bombs */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Can AI Spawn Bomb"))
	FORCEINLINE bool CanAISpawnBomb() const { return bCanSpawnBombs; }

	/** Enable or disable spawning bombs for this bot (might be useful for some game modes)
	 * Main logic still will be running unless move input is disabled as well. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (DisplayName = "Set AI Can Spawn Bomb"))
	void SetAICanSpawnBomb(bool bCanSpawn) { bCanSpawnBombs = bCanSpawn; }

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Last time AI was updated, used to control update frequency, once per UGameStateDataAsset::TickInternal. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Update Handle"))
	float LastAIUpdateTimeInternal = 0.f;

	/** Cell position of current path segment's end */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Move To"))
	FCell AIMoveToInternal = FCell::InvalidCell;

	/** If disabled, AI will not be able to put any bombs. Might be useful for some game modes.
	 * Main logic still will be running unless move input is disabled as well. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Can Spawn Bombs"))
	bool bCanSpawnBombs = true;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** This is called only in the gameplay before calling nplan play. */
	virtual void PostInitializeComponents() override;

	/** Allows the controller to react on possessing the pawn to enable AI. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Allows the controller to react on unpossessing the pawn to disable AI. */
	virtual void OnUnPossess() override;

	/** Stops running to target. */
	virtual void Reset() override;

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateAI();

	/** Enable or disable AI for this bot. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (BlueprintProtected))
	void SetAI(bool bShouldEnable);

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Listen game states to enable or disable AI. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when this level actor is destroyed on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(class UMapComponent* MapComponent, UObject* DestroyCauser);

	/** Called when owner's movement is completed for the time step. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnOwnerMovementCompleted(const struct FMoverTimeStep& TimeStep);
};