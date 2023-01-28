// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/CheatManager.h"
//---
#include "SettingsCheatExtension.generated.h"

/**
 * Automatically extends any cheat manager with settings-related console commands.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingsCheatExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	USettingsCheatExtension();

	/** Register a delegate to call whenever a cheat manager is spawned; it will also be called immediately for cheat managers that already exist at this point. */
	void OnCheatManagerCreated(UCheatManager* CheatManager);

	/** Override the setting value with the cheat.
	 * [Types]			[Example command]
	 * Button:			CheatSetting Close
	 * Checkbox:		CheatSetting VSync?1
	 * Combobox index:	CheatSetting Shadows?2
	 * Combobox list:	CheatSetting Shadows?Low,Medium,High
	 * Slider:			CheatSetting Audio?0.5
	 * Text Line:		CheatSetting Title?Settings
	 * User Input:		CheatSetting Player?JanSeliv
	 *
	 * @param TagByValue Tag?Value */
	UFUNCTION(Exec)
	void CheatSetting(const FString& TagByValue) const;
};
