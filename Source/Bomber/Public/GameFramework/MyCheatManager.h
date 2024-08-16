// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MetaCheatManager.h"
//---
#include "HAL/IConsoleManager.h" // TAutoConsoleVariable
//---
#include "MyCheatManager.generated.h"

enum class EActorType : uint8;

/**
 * Contains debugging cheat command for non-shipping builds to test general game functionality.
 */
UCLASS()
class BOMBER_API UMyCheatManager final : public UMetaCheatManager
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UMyCheatManager();

	/*********************************************************************************************
	 * Utils
	 ********************************************************************************************* */
public:
	/**
	 * Returns bitmask from reverse bitmask in string.
	 * "1000"(OR "1 0 0 0" OR "1")	-> 1,
	 * "1100"(OR "1 1 0 0" OR "11") -> 3,
	 * "0011"(OR "0 0 1 1")			-> 12,
	 * "0001"(OR "0 0 0 1") 		-> 8
	 * "0001"(OR "1 1 1 1") 		-> 15
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ReverseBitmaskStr"))
	static int32 GetBitmaskFromReverseString(const FString& ReverseBitmaskStr);

	/**
	 * Returns bitmask from actor types in string.
	 * "Wall" (1<<4)						-> 16,
	 * "Wall Bomb" (1<<4|1<<0)				-> 17,
	 * "Wall Bomb Player" (1<<4|1<<0|1<<3)	-> 25,
	 */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ActorTypesBitmaskStr"))
	static int32 GetBitmaskFromActorTypesString(const FString& ActorTypesBitmaskStr);

	/*********************************************************************************************
	 * Destroy
	 ********************************************************************************************* */
public:
	/**
	 * Destroy all specified level actors on the map.
	 * @param ActorType Bomb, Box, Item, Player, Wall, All
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Destroy.AllByType"))
	static void DestroyAllByType(EActorType ActorType);

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
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Destroy.PlayersBySlots"))
	static void DestroyPlayersBySlots(const FString& Slot);

	/*********************************************************************************************
	 * Box
	 ********************************************************************************************* */
public:
	/**
	 * Override the chance to spawn item after box destroying.
	 * @param Chance Put 0 to stop spawning, 100 to spawn all time.
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Box.SetItemChance"))
	static void SetItemChance(int32 Chance);

	/*********************************************************************************************
	 * Player
	 ********************************************************************************************* */
public:
	/**
	 * Override the level of each powerup for a controlled player.
	 * @param NewLevel 1 is minimum, 5 is maximum.
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Player.SetPowerups"))
	static void SetPlayerPowerups(int32 NewLevel);

	/** Enable or disable the God mode to make a controllable player undestroyable. */
	UFUNCTION(meta = (CheatName = "Bomber.Player.SetGodMode"))
	static void SetGodMode(bool bShouldEnable);

	/** Enable or disable the Auto Copilot mode to make a controllable player to play automatically. */
	UFUNCTION(meta = (CheatName = "Bomber.Player.SetAutoCopilot"))
	static void SetAutoCopilot();

	/*********************************************************************************************
	 * AI
	 ********************************************************************************************* */
public:
	/** Enable or disable all bots.
	 * Bomber.AI.SetEnabled 0 - disable all bots. */
	static TAutoConsoleVariable<bool> CVarAISetEnabled;

	/**
	 * Override the level of each powerup for bots.
	 * @param NewLevel 1 is minimum, 5 is maximum.
	 */
	UFUNCTION(meta = (CheatName = "Bomber.AI.SetPowerups"))
	static void SetAIPowerups(int32 NewLevel);

	/** If called, all bots will change own skin to look like players.
	 * Cheat is called without parameters. */
	UFUNCTION(meta = (CheatName = "Bomber.AI.ApplyPlayersSkin"))
	static void ApplyPlayersSkinOnAI();

	/** Spawns additional bot at the center of the level.
	 * It will add as many bots as called with no limit to the number of bots.
	 * Cheat is called without parameters. */
	UFUNCTION(meta = (CheatName = "Bomber.AI.AddBot"))
	static void AddBot();

	/*********************************************************************************************
	 * Debug
	 ********************************************************************************************* */
public:
	/**
	 * Shows coordinates of all level actors by specified types, ex: 'Box Item'.
	 * Bomber.Debug.DisplayCells Wall - show walls.
	 * Bomber.Debug.DisplayCells Wall Bomb - show walls and bombs.
	 * Bomber.Debug.DisplayCells Wall Bomb Player - show walls, bombs and players.
	 * Bomber.Debug.DisplayCells All - show all actors (walls, bombs, players, items and boxes).
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Debug.DisplayCells"))
	static void DisplayCells(const FString& ActorTypesString);

	/*********************************************************************************************
	 * Level
	 ********************************************************************************************* */
public:
	/** Sets the size for generated map, it will automatically regenerate the level for given size.
	 * @see LevelSize The new size where length and width have to be unpaired (odd).
	 * Bomber.Level.SetSize 9x7 - set the size of the level to 9 columns (width) and 7 rows (length).
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SetSize"))
	static void SetLevelSize(const FString& LevelSize);

	/**
	 * Spawns an actor by type on the level. 
	 * @param ActorType The type of actor to spawn: Bomb, Box, Item, Player, Wall
	 * @param ColumnX Position on the X-axis by column: 0, 1... N
	 * @param RowY  Position on the Y-axis by row: 0, 1... N
	 * @param RowIndex The row from Data Assets to select across different variants of the same actor type: 0, 1... N
	 * Bomber.Level.SpawnActorByType Bomb 1 1 0 - spawn a bomb at the position (1, 1) with the first variant of the bomb (row 0 - Maya).
	 * Bomber.Level.SpawnActorByType Player 2 2 4 - spawn a player at the position (2, 2) with the fifth variant of the player (row 4 - AI). 
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SpawnActorByType"))
	static void SpawnActorByType(EActorType ActorType, int32 ColumnX, int32 RowY, int32 RowIndex);

	/*********************************************************************************************
	 * Camera
	 ********************************************************************************************* */
public:
	/** Is overridden to let internal systems know that camera manager is enabled.
	 * Is commonly called from `ToggleDebugCamera` cheat. */
	virtual void EnableDebugCamera() override;

	/** Is overridden to let internal systems know that camera manager is disabled.
	 * Is commonly called from `ToggleDebugCamera` cheat. */
	virtual void DisableDebugCamera() override;

	/** Tweak the custom additive angle to affect the fit distance calculation from camera to the level. */
	UFUNCTION(meta = (CheatName = "Bomber.Camera.FitViewAdditiveAngle"))
	static void FitViewAdditiveAngle(float InFitViewAdditiveAngle);

	/** Tweak the minimal distance in UU from camera to the level. */
	UFUNCTION(meta = (CheatName = "Bomber.Camera.MinDistance"))
	static void MinDistance(float InMinDistance);

	/*********************************************************************************************
	 * UI
	 ********************************************************************************************* */
public:
	/** Completely removes all widgets from UI. */
	UFUNCTION(meta = (CheatName = "Bomber.UI.HideAllWidgets"))
	static void SetUIHideAllWidgets();
};