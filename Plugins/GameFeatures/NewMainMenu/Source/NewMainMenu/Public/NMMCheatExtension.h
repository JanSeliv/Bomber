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
	UFUNCTION(Exec, meta = (CheatName = "Bomber.Saves.NewMainMenu.Reset"))
	static void ResetNewMainMenuSaves();
};
