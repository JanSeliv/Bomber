// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "NMMInGameSettingsSubsystem.generated.h"

/**
 * Contains Main Menu settings that are tweaked by player in Settings menu during the game.
 */
UCLASS(BlueprintType, Blueprintable, Config = "GameUserSettings", DefaultConfig)
class NEWMAINMENU_API UNMMInGameSettingsSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** Returns this Subsystem, is checked and wil crash if can't be obtained.*/
	static UNMMInGameSettingsSubsystem& Get();

	/** Returns true is setting enabled to skips previously seen cinematics automatically.
	 * @warning in multiplayer, this setting is ignored, so cinematics are always skipped. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE bool IsAutoSkipCinematicsEnabled() const { return bAutoSkipCinematics; }

	/** Set true to skip previously seen cinematics automatically.
	 * Is called from Settings menu once its checkbox is changed. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetAutoSkipCinematics(bool bEnable);

	/** Set new sound volume for Cinematics sound class.
	 * Is called from Settings menu once Cinematics slider is changed.*/
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetCinematicsVolume(double InVolume);

	/** Returns the sound volume for Cinematics sound class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE double GetCinematicsVolume() const { return CinematicsVolume; }

	/** Returns true if enabled instant transitions when switching characters in the Main Menu. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[NewMainMenu]")
	FORCEINLINE bool IsInstantCharacterSwitchEnabled() const { return bInstantCharacterSwitch; }

	/** Set true to enable instant transitions when switching characters in the main menu.
	 * Is called from Settings menu once its checkbox is changed. */
	UFUNCTION(BlueprintCallable, Category = "[NewMainMenu]")
	void SetInstantCharacterSwitchEnabled(bool bEnable);

protected:
	/** When setting enabled, skips previously seen cinematics automatically.
	 * Is config property, can be set in Settings menu.
	 * @warning in multiplayer, this setting is ignored, so cinematics are always skipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	bool bAutoSkipCinematics = true;

	/** The sound volume for Cinematics sound class.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	double CinematicsVolume = 1.f;

	/** Enable/disable smooth transitions when switching characters in the main menu.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[NewMainMenu]", meta = (BlueprintProtected))
	bool bInstantCharacterSwitch = false;
};
