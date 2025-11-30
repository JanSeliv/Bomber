// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"

#include "BmrGameUserSettings.generated.h"

/**
 * Player settings that can be changed in the game settings menu.
 * It contains only video settings while all gameplay settings are stored right in owner classes (like GameDifficultySubsystem).
 * @see DefaultGameUserSettings.ini
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrGameUserSettings final : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** Returns the game user settings.
	 * Is init once and can not be destroyed. */
	static UBmrGameUserSettings& Get();

	/*********************************************************************************************
	 * Delegates
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveSettings);

	/** Called when the settings were saved. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnSaveSettings OnSaveSettings;

	/*********************************************************************************************
	 * Resolution
	 * In base class: GetScreenResolution()
	 ********************************************************************************************* */
public:
	/** Returns the index of chosen resolution*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetResolutionIndex() const { return CurrentResolutionIndex; }

	/** Returns the min allowed resolution width. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetMinResolutionSizeX() const { return MinResolutionSizeX; }

	/** Returns the min allowed resolution height. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetMinResolutionSizeY() const { return MinResolutionSizeY; }

	/** Get all supported resolutions of the primary monitor in the text format. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	void GetTextResolutions(TArray<FText>& OutTextResolutions) const { OutTextResolutions = TextResolutions; }

	/** Get all supported resolutions of the primary monitor in the int point format.. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	void GetIntResolutions(TArray<FIntPoint>& OutIntResolutions) const { OutIntResolutions = IntResolutions; }

	/** Call to update supported resolutions in arrays. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateSupportedResolutions();

	/** Set and apply a new resolution by index. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetResolutionByIndex(int32 Index);

	/** Syncs the current resolution index with the actual resolution, is useful when it's changed outside (e.g. by Alt+Enter). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TryUpdateCurrentResolution();

protected:
	/** The min allowed resolution width.
	 * Is set on starting from game (not settings) config.
	 * By default is 1280. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 MinResolutionSizeX = 1280;

	/** The min allowed resolution height.
	 * Is set on starting from game (not settings) config.
	 * By default is 720. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 MinResolutionSizeY = 720;

	/** Contains all resolutions. Is displayed on UI. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<FText> TextResolutions;

	/** Contains all resolutions in the int point format. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<FIntPoint> IntResolutions;

	/** The index of chosen resolution. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 CurrentResolutionIndex = 0;

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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static FORCEINLINE EWindowMode::Type GetSupportedWindowModeType(bool bReturnFullscreen) { return bReturnFullscreen ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed; }

	/** Returns true if the game is in fullscreen mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsFullscreenEnabled() const { return GetFullscreenMode() == GetSupportedWindowModeType(/*bReturnFullscreen*/ true); }

	/** Set and apply fullscreen mode. If false, the windowed mode will be applied. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetFullscreenEnabled(bool bIsFullscreen);

	/** Syncs the current Fullscreen mode with the actual mode, is useful when it's changed outside (e.g. by Alt+Enter). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TryUpdateCurrentFullscreenMode();

	/*********************************************************************************************
	 * FPS Lock
	 ********************************************************************************************* */
protected:
	/** Returns the index of chosen fps lock in array. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetFPSLockIndex() const { return FPSLockIndex; }

	/** Set the FPS cap by specified member index. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetFPSLockByIndex(int32 Index);

protected:
	/** The index of chosen fps lock in array, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 FPSLockIndex;

	/*********************************************************************************************
	 * Overall Quality (Scalability)
	 ********************************************************************************************* */
public:
	/* Returns the overall scalability level, is declared in parent as UFUNCTION. */
	virtual int32 GetOverallScalabilityLevel() const override;

	/** Changes all scalability settings at once based on a single overall quality level, is declared in parent as UFUNCTION.
	 * @param Value New quality level.
	 * @see UBmrGameUserSettings::OverallQuality */
	virtual void SetOverallScalabilityLevel(int32 Value) override;

	/** Is called to apply the currently chosen overall quality level. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyOverallScalabilityLevel();

protected:
	/** The overall quality level, is config property.
	 * 0:custom, 1:low, 2:medium, 3:high, 4:very high, 5:ultra. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 OverallQuality;

	/*********************************************************************************************
	 * Language
	 ********************************************************************************************* */
public:
	/** Returns the index of chosen language */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetLanguageIndex() const { return CurrentLanguageIndex; }

	/** Get all supported languages in text format. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	void GetTextLanguages(TArray<FText>& OutTextLanguages) const { OutTextLanguages = DisplayLanguages; }

	/** Set and apply a new language by index. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetLanguageByIndex(int32 Index);

	/** Is called to apply the currently chosen language. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyCurrentLanguage();

	/** Call to update supported languages in arrays. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void UpdateSupportedLanguages();

protected:
	/** Contains all available languages in their native names to display on UI.
	 * Order is in sync with Cultures. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<FText> DisplayLanguages;

	/** Contains all available cultures available to apply: en, ru, etc.
	 * Order is in sync with DisplayLanguages. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<FName> Cultures;

	/** The index of chosen language. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 CurrentLanguageIndex = INDEX_NONE;

	/** Is currently selected culture (e.g: 'en'), is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FName AppliedCulture;

	/*********************************************************************************************
	 * FPS Counter
	 ********************************************************************************************* */
public:
	/** Set true to show the FPS counter widget on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetFPSCounterEnabled(bool bEnable);

	/** Returns true if the FPS counter widget is shown on the HUD. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsFPSCounterEnabled() const { return bIsFPSCounterEnabled; }

protected:
	/** If true, shows FPS counter widget on the HUD, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bIsFPSCounterEnabled;

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