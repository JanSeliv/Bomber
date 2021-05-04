// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameUserSettings.h"
//---
#include "Structures/SettingsRow.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "MyGameUserSettings.generated.h"

/**
 * Describes common data of settings.
 */
UCLASS()
class BOMBER_API USettingsDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the settings data asset. */
	static const USettingsDataAsset& Get();

	/** Returns the table rows.
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GenerateSettingsArray(TMap<FName, FSettingsPicker>& OutRows) const;

	/** Delegate to react on changing settings data table. */
	DECLARE_DYNAMIC_DELEGATE(FOnDataTableChanged);

	/** Get a multicast delegate that is called any time the data table changes.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, BlueprintPure = "false", Category = "C++", meta = (DevelopmentOnly))
	void BindOnDataTableChanged(const FOnDataTableChanged& EventToBind) const;

	/** Returns the data table. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UDataTable* GetSettingsDataTable() const { return SettingsDataTableInternal; }

	/** Returns the width of the widget.
	 * @see USettingsDataAsset::WidthByScreen */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetWidthByScreen() const { return WidthByScreenInternal; }

	/** Returns the height of the widget.
	 * @see USettingsDataAsset::HeightByScreen */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetHeightByScreen() const { return HeightByScreenInternal; }

	/** Returns the title name of the widget.
	 * @see USettingsDataAsset::Title */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FText GetTitle() const { return TitleInternal; }

protected:
	/** The data table with all settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Data Table", ShowOnlyInnerProperties))
	class UDataTable* SettingsDataTableInternal; //[D]

	/** The width of the widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Width By Screen", ShowOnlyInnerProperties))
	float WidthByScreenInternal; //[D]

	/** The height of the widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Height By Screen", ShowOnlyInnerProperties))
	float HeightByScreenInternal; //[D]

	/** The title name of the widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Title", ShowOnlyInnerProperties))
	FText TitleInternal; //[D]
};

/**
 * The Bomber settings.
 */
UCLASS(Blueprintable, BlueprintType)
class UMyGameUserSettings final : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Returns the game user settings.
	 * Is init once and can not be destroyed. */
	static UMyGameUserSettings& Get();

	/** Returns the settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class USettingsWidget* GetSettingsWidget() const;

protected:
	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Loads the user settings from persistent storage */
	virtual void LoadSettings(bool bForceReload) override;

	/** Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks.
	 * @warning DevelopmentOnly */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, DevelopmentOnly))
	void OnDataTableChanged();
};
