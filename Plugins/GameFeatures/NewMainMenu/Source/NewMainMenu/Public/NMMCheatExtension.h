// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MetaCheatManagerExtension.h"
//---
#include "NMMCheatExtension.generated.h"

/**
 * Extends cheat manager with New Main Menu-related console commands.
 */
UCLASS()
class NEWMAINMENU_API UNMMCheatExtension : public UMetaCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** Removes a save file of the New Main Menu. */
	UFUNCTION(Exec, meta = (CheatName = "Bomber.Saves.Reset.NewMainMenu"))
	static void ResetNewMainMenuSaves();

	/** Sets player skins availability by slots 
	 * Each slot represents a skin, where 1 means available and 0 means locked.
	 * Bomber.Player.SetSkinsAvailable 1100 - set the first and second skins available, the third and fourth are locked. */
	UFUNCTION(meta = (CheatName = "Bomber.Player.SetSkinsAvailable"))
	static void SetPlayerSkinsAvailable(const FString& SkinAvailabilityMask);
};
