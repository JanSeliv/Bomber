// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Subsystems/EngineSubsystem.h"
//---
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
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsAutoSkipCinematicsEnabled() const { return bAutoSkipCinematicsInternal; }

	/** Set true to skip previously seen cinematics automatically.
	 * Is called from Settings menu once its checkbox is changed. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetAutoSkipCinematics(bool bEnable);

	/** Set new sound volume for Cinematics sound class.
	 * Is called from Settings menu once Cinematics slider is changed.*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCinematicsVolume(double InVolume);

	/** Returns the sound volume for Cinematics sound class. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE double GetCinematicsVolume() const { return CinematicsVolumeInternal; }

	/** Returns true if enabled instant transitions when switching characters in the Main Menu. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsInstantCharacterSwitchEnabled() const { return bInstantCharacterSwitchInternal; }

	/** Set true to enable instant transitions when switching characters in the main menu.
	 * Is called from Settings menu once its checkbox is changed. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetInstantCharacterSwitchEnabled(bool bEnable);

protected:
	/** When setting enabled, skips previously seen cinematics automatically.
	 * Is config property, can be set in Settings menu.
	 * @warning in multiplayer, this setting is ignored, so cinematics are always skipped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Auto Skip Cinematics"))
	bool bAutoSkipCinematicsInternal = true;

	/** The sound volume for Cinematics sound class.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Music Volume"))
	double CinematicsVolumeInternal = 1.f;

	/** Enable/disable smooth transitions when switching characters in the main menu.
	 * Is config property, can be set in Settings menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Instant Character Switch"))
	bool bInstantCharacterSwitchInternal = false;
};
