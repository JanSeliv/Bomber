// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"
//---
#include "MyGameUserSettings.generated.h"

/**
 * The Bomber settings.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UMyGameUserSettings final : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Returns the game user settings.
	 * Is init once and can not be destroyed. */
	static UMyGameUserSettings& Get();

	/** Validates and resets bad user settings to default. Deletes stale user settings file if necessary. */
	virtual void ValidateSettings() override;

	/** Changes all scalability settings at once based on a single overall quality level, is declared in parent as UFUNCTION.
	 * @param Value New quality level.
	 * @see UMyGameUserSettings::OverallQualityInternal */
	virtual void SetOverallScalabilityLevel(int32 Value) override;

	/* Returns the overall scalability level, is declared in parent as UFUNCTION. */
	virtual int32 GetOverallScalabilityLevel() const override;

	/** Mark current video mode settings (fullscreenmode/resolution) as being confirmed by the user. */
	virtual void ConfirmVideoMode() override;

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

	/** Returns the index of chosen resolution*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetResolutionIndex() const { return CurrentResolutionIndexInternal; }

	/** Returns true if the game is in fullscreen mode. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFullscreenEnabled() const { return GetFullscreenMode() == EWindowMode::Fullscreen; }

	/** Set and apply fullscreen mode. If false, the windowed mode will be applied. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFullscreenEnabled(bool bIsFullscreen);

	/** Update fullscreen mode on UI for cases when it's changed outside (e.g. by Alt+Enter). */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateFullscreenEnabled();

	/** Returns the index of chosen fps lock in array. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetFPSLockIndex() const { return FPSLockIndexInternal; }

	/** Set the FPS cap by specified member index. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSLockByIndex(int32 Index);

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The overall quality level, is config property.
	 * 0:custom, 1:low, 2:medium, 3:high, 4:very high, 5:ultra. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Overall Quality"))
	int32 OverallQualityInternal;

	/** The min allowed resolution width.
	 * Is set on starting from game (not settings) config.
	 * By default is 1280. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Min Resolution Size X"))
	int32 MinResolutionSizeXInternal = 1280;

	/** The min allowed resolution height.
	 * Is set on starting from game (not settings) config.
	 * By default is 720. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Min Resolution Size Y"))
	int32 MinResolutionSizeYInternal = 720;

	/** Contains all resolutions. Is displayed on UI. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Text Resolutions"))
	TArray<FText> TextResolutionsInternal;

	/** Contains all resolutions in the int point format. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Int Resolutions"))
	TArray<FIntPoint> IntResolutionsInternal;

	/** The index of chosen resolution. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Resolution Index"))
	int32 CurrentResolutionIndexInternal = 0;

	/** The index of chosen fps lock in array, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Lock Index"))
	int32 FPSLockIndexInternal;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Loads the user settings from persistent storage */
	virtual void LoadSettings(bool bForceReload) override;
};
