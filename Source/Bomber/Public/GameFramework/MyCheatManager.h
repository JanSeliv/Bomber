// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/CheatManager.h"
#include "Bomber.h"
//---
#include "MyCheatManager.generated.h"

/**
 * Contains debugging code for non-shipping builds to check general game functionality.
 */
UCLASS()
class BOMBER_API UMyCheatManager : public UCheatManager
{
	GENERATED_BODY()

protected:
	/** Called when CheatManager is created to allow any needed initialization. */
	virtual void InitCheatManager() override;

	/**
	 * Returns bitmask by string.
	 * "1000"(OR "1 0 0 0" OR "1")	-> 1,
	 * "1100"(OR "1 1 0 0" OR "11") -> 3,
	 * "0011"(OR "0 0 1 1")			-> 12,
	 * "0001"(OR "0 0 0 1") 		-> 8
	 * "0001"(OR "1 1 1 1") 		-> 15
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "String"))
	int32 GetBitmask(const FString& String) const;

	/* ---------------------------------------------------
	*		Destroy
	* --------------------------------------------------- */

	/**
	 * Destroy all specified level actors on the map.
	 * @param ActorType Bomb, Box, Item, Player, Wall, All
	 */
	UFUNCTION(Exec, meta = (OverrideNativeName = "Bomber.Destroy.AllByType"))
	void DestroyAllByType(EActorType ActorType) const;

	/**
	 * Destroy characters in specified slots.
	 * ID: 0 1 2 3 - ID of each character, where #0 is player, #1, #2, #3 are bots
	 * Ex: 1 1 1 1 - destroy player and all bots (get Draw)
	 * Ex: 1 0 0 0 - destroy player (get Lose)
	 * Ex: 0 1 1 1 - destroy bots (get Win)
	 * Ex: 0 0 1 0 - destroy bot with ID #2
	 * Ex: 1 0 0 1 - destroy player + bot with id #3 (get Lose)
	 * Ex: 0 0 1 1 - destroy bots with ID #2 and #3
	 * Bomber.Destroy.PlayersBySlots 1111
	 * Bomber.Destroy.PlayersBySlots 1000
	 * Bomber.Destroy.PlayersBySlots 0111
	 * @param Slot 1 if should destroy
	 */
	UFUNCTION(Exec, meta = (OverrideNativeName = "Bomber.Destroy.PlayersBySlots"))
	void DestroyPlayersBySlots(const FString& Slot) const;

	/* ---------------------------------------------------
	*		Box
	* --------------------------------------------------- */

	/**
	 * Override the chance to spawn item after box destroying.
	 * @param Chance Put 0 to stop spawning, 100 to spawn all time.
	 */
	UFUNCTION(Exec, meta = (OverrideNativeName = "Bomber.Box.SetItemChance"))
	void SetItemChance(int32 Chance) const;

	/* ---------------------------------------------------
	*		Player
	* --------------------------------------------------- */

	/**
	 * Override the level of each powerup for a controlled player.
	 * @param NewLevel 1 is minimum, 9 is maximum.
	 */
	UFUNCTION(Exec, meta = (OverrideNativeName = "Bomber.Player.SetPowerups"))
	void SetPowerups(int32 NewLevel) const;

	/** Enable or disable the God mode to make a controllable player undestroyable. */
	UFUNCTION(Exec, meta = (OverrideNativeName = "Bomber.Player.SetGodMode"))
	void SetGodMode(bool bShouldEnable) const;
};
