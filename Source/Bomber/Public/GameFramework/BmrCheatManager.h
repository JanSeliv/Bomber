// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MetaCheatManager.h"

// UE
#include "HAL/IConsoleManager.h" // TAutoConsoleVariable

#include "BmrCheatManager.generated.h"

enum class EBmrActorType : uint8;
enum class EBmrCurrentGameState : uint8;

/**
 * Contains debugging cheat command for non-shipping builds to test general game functionality.
 */
UCLASS()
class BOMBER_API UBmrCheatManager final : public UMetaCheatManager
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrCheatManager();

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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "ReverseBitmaskStr"))
	static int32 GetBitmaskFromReverseString(const FString& ReverseBitmaskStr);

	/**
	 * Returns bitmask from actor types in string.
	 * "Wall" (1<<4)						-> 16,
	 * "Wall Bomb" (1<<4|1<<0)				-> 17,
	 * "Wall Bomb Player" (1<<4|1<<0|1<<3)	-> 25,
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]", meta = (AutoCreateRefTerm = "ActorTypesBitmaskStr"))
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
	static void DestroyAllByType(EBmrActorType ActorType);

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
	/** Override the percentage of items spawn from boxes, where 100 is maximum, 0 is disabled (default chance will be used).
	 * e.g Bomber.Box.SetPowerupsChance 100 - set 100% chance to spawn powerups. */
	static TAutoConsoleVariable<int32> CVarPowerupsChance;

	/*********************************************************************************************
	 * Player
	 ********************************************************************************************* */
public:
	/** Is overridden to apply damage immunity effect for proper integration with Ability System. */
	virtual void God() override;

	/** Forcing locally controlled player to destroy itself immediately, resulting in loss.
	 * Cheat is called without parameters. */
	UFUNCTION(meta = (CheatName = "Bomber.Player.Suicide"))
	static void Suicide();

	/**
	 * Override the level of each powerup for a controlled player.
	 * @param NewLevel 1 is minimum, 5 is maximum.
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Player.SetPowerups"))
	static void SetPlayerPowerups(int32 NewLevel);

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
	/** Shows coordinates of level actors of specified types (requires regeneration), e.g: 'Box Item'.
	 * Bomber.Debug.DisplayCells Wall - show walls.
	 * Bomber.Debug.DisplayCells Wall Bomb - show walls and bombs.
	 * Bomber.Debug.DisplayCells Wall Bomb Player - show walls, bombs and players.
	 * Bomber.Debug.DisplayCells All - show all actors (walls, bombs, players, items and boxes). */
	static TAutoConsoleVariable<FString> CVarDisplayCells;

	/*********************************************************************************************
	 * Level
	 ********************************************************************************************* */
public:
	/** Sets the size for generated map, it will automatically regenerate the level for given size.
	 * @param LevelSize The new size where length and width have to be unpaired (odd).
	 * Bomber.Level.SetSize 9x7 - set the size of the level to 9 columns (width) and 7 rows (length). */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SetSize"))
	static void SetLevelSize(const FString& LevelSize);

	/**
	 * Spawns an actor by type on the level.
	 * @param ActorType The type of actor to spawn: Bomb, Box, Item, Player, Wall
	 * @param ColumnX Position on the X-axis by column: 0, 1... N
	 * @param RowY  Position on the Y-axis by row: 0, 1... N
	 * @param SkinIndex The row from Data Assets to select across different variants of the same actor type: 0, 1... N
	 * Bomber.Level.SpawnActorByType Bomb 1 1 0 - spawn a bomb at the position (1, 1) with the first variant of the bomb (row 0 - Maya).
	 * Bomber.Level.SpawnActorByType Player 2 2 4 - spawn a player at the position (2, 2) with the fifth variant of the player (row 4 - AI).
	 */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SpawnActorByType"))
	static void SpawnActorByType(EBmrActorType ActorType, int32 ColumnX, int32 RowY, int32 SkinIndex);

	/** Overrides the percentage of walls spawn during the level generation, it will automatically regenerate the level for given chance.
	 * @param WallsChance The new walls spawn chance where 100 is maximum, 0 is empty level with no walls, -1 will use the chance from GeneratedMapSettings.
	 * Bomber.Level.SetWallsChance 100 - set 100% chance to spawn walls. */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SetWallsChance"))
	static void SetWallsChance(int32 WallsChance);

	/** Overrides the percentage of boxes spawn during the level generation, it will automatically regenerate the level for given chance.
	 * @param BoxesChance The new boxes spawn chance where 100 is maximum, 0 is empty level with no boxes, -1 will use the chance from GeneratedMapSettings.
	 * Bomber.Level.SetBoxesChance 100 - set 100% chance to spawn boxes. */
	UFUNCTION(meta = (CheatName = "Bomber.Level.SetBoxesChance"))
	static void SetBoxesChance(int32 BoxesChance);

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

	/*********************************************************************************************
	 * Game
	 ********************************************************************************************* */
public:
	/** Sets current game state to the specified one.
	 * @param GameState The state of the game to set: Menu, GameStarting, EndGame, InGame
	 * Is useful to trigger different game states skipping default transitions.
	 * Bomber.Game.SetGameState InGame - the match will be started immediately without any countdown. */
	UFUNCTION(meta = (CheatName = "Bomber.Game.SetGameState"))
	static void SetGameState(EBmrCurrentGameState GameState);
};