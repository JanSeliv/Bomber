// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"
//---
#include "MyGameUserSettings.generated.h"

/**
 * Player settings that can be changed in the game settings menu.
 * It contains only video settings while all gameplay settings are stored right in owner classes (like GameDifficultySubsystem).
 * @see DefaultGameUserSettings.ini
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UMyGameUserSettings final : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** Returns the game user settings.
	 * Is init once and can not be destroyed. */
	static UMyGameUserSettings& Get();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveSettings);

	/** Called when the settings were saved. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnSaveSettings OnSaveSettings;

	/*********************************************************************************************
	 * Resolution
	 * In base class: GetScreenResolution()
	 ********************************************************************************************* */
public:
	/** Returns the index of chosen resolution*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetResolutionIndex() const { return CurrentResolutionIndexInternal; }

	/** Returns the min allowed resolution width. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMinResolutionSizeX() const { return MinResolutionSizeXInternal; }

	/** Returns the min allowed resolution height. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMinResolutionSizeY() const { return MinResolutionSizeYInternal; }

	/** Get all supported resolutions of the primary monitor in the text format. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetTextResolutions(TArray<FText>& OutTextResolutions) const { OutTextResolutions = TextResolutionsInternal; }

	/** Get all supported resolutions of the primary monitor in the int point format.. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetIntResolutions(TArray<FIntPoint>& OutIntResolutions) const { OutIntResolutions = IntResolutionsInternal; }

	/** Call to update supported resolutions in arrays. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateSupportedResolutions();

	/** Set and apply a new resolution by index. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetResolutionByIndex(int32 Index);

	/** Syncs the current resolution index with the actual resolution, is useful when it's changed outside (e.g. by Alt+Enter). */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TryUpdateCurrentResolution();

protected:
	/** The min allowed resolution width.
	 * Is set on starting from game (not settings) config.
	 * By default is 1280. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Min Resolution Size X"))
	int32 MinResolutionSizeXInternal = 1280;

	/** The min allowed resolution height.
	 * Is set on starting from game (not settings) config.
	 * By default is 720. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Min Resolution Size Y"))
	int32 MinResolutionSizeYInternal = 720;

	/** Contains all resolutions. Is displayed on UI. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Text Resolutions"))
	TArray<FText> TextResolutionsInternal;

	/** Contains all resolutions in the int point format. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Int Resolutions"))
	TArray<FIntPoint> IntResolutionsInternal;

	/** The index of chosen resolution. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Resolution Index"))
	int32 CurrentResolutionIndexInternal = 0;

	/*********************************************************************************************
	 * Fullscreen
	 * In base class: GetFullscreenMode()
	 ********************************************************************************************* */
public:
	/** Returns the enum type of supported fullscreen mode.
	 * Is expanded as a function to avoid usage of unsupported modes.
	 * Never use directly EWindowMode type, but only this function. 
	 * @param bReturnFullscreen If true, then EWindowMode::WindowedFullscreen will be returned, otherwise EWindowMode::Windowed.
	 * @warning Native Fullscreen (EWindowMode::Fullscreen) is not supported at all because of various engine issues, WindowedFullscreen is used instead. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static FORCEINLINE EWindowMode::Type GetSupportedWindowModeType(bool bReturnFullscreen) { return bReturnFullscreen ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed; }

	/** Returns true if the game is in fullscreen mode. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFullscreenEnabled() const { return GetFullscreenMode() == GetSupportedWindowModeType(/*bReturnFullscreen*/true); }

	/** Set and apply fullscreen mode. If false, the windowed mode will be applied. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFullscreenEnabled(bool bIsFullscreen);

	/** Syncs the current Fullscreen mode with the actual mode, is useful when it's changed outside (e.g. by Alt+Enter). */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TryUpdateCurrentFullscreenMode();

	/*********************************************************************************************
	 * FPS Lock
	 ********************************************************************************************* */
protected:
	/** Returns the index of chosen fps lock in array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetFPSLockIndex() const { return FPSLockIndexInternal; }

	/** Set the FPS cap by specified member index. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSLockByIndex(int32 Index);

protected:
	/** The index of chosen fps lock in array, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Lock Index"))
	int32 FPSLockIndexInternal;

	/*********************************************************************************************
	 * Overall Quality (Scalability)
	 ********************************************************************************************* */
public:
	/* Returns the overall scalability level, is declared in parent as UFUNCTION. */
	virtual int32 GetOverallScalabilityLevel() const override;

	/** Changes all scalability settings at once based on a single overall quality level, is declared in parent as UFUNCTION.
	 * @param Value New quality level.
	 * @see UMyGameUserSettings::OverallQualityInternal */
	virtual void SetOverallScalabilityLevel(int32 Value) override;

protected:
	/** The overall quality level, is config property.
	 * 0:custom, 1:low, 2:medium, 3:high, 4:very high, 5:ultra. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Overall Quality"))
	int32 OverallQualityInternal;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Loads the user settings from persistent storage */
	virtual void LoadSettings(bool bForceReload) override;

	/** Validates and resets bad user settings to default. Deletes stale user settings file if necessary. */
	virtual void ValidateSettings() override;

	/** Save the user settings to persistent storage (automatically happens as part of ApplySettings). */
	virtual void SaveSettings() override;

	/** Mark current video mode settings (fullscreenmode/resolution) as being confirmed by the user. */
	virtual void ConfirmVideoMode() override;
};