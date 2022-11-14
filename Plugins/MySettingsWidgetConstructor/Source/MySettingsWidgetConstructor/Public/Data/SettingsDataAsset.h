// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "Structures/SettingsRow.h"
#include "SettingsDataTable.h"
//---
#include "SettingsDataAsset.generated.h"

/**
 * Describes common data of settings.
 */
UCLASS(Config = MySettingsWidgetConstructor, DefaultConfig)
class MYSETTINGSWIDGETCONSTRUCTOR_API USettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor.  */
	USettingsDataAsset();

	/** Returns the data table. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE USettingsDataTable* GetSettingsDataTable() const { return SettingsDataTableInternal.LoadSynchronous(); }

	/** Returns the sub-widget of Button settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingButton> GetButtonClass() const { return ButtonClassInternal; }

	/** Returns the sub-widget of Checkbox settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingCheckbox> GetCheckboxClass() const { return CheckboxClassInternal; }

	/** Returns the sub-widget of Combobox settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingCombobox> GetComboboxClass() const { return ComboboxClassInternal; }

	/** Returns the sub-widget of Slider settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingSlider> GetSliderClass() const { return SliderClassInternal; }

	/** Returns the sub-widget of Text Line settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingTextLine> GetTextLineClass() const { return TextLineClassInternal; }

	/** Returns the sub-widget of User Input settings. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingUserInput> GetUserInputClass() const { return UserInputClassInternal; }

	/** Returns the width and height of the settings widget in percentages of an entire screen. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FVector2D& GetSettingsPercentSize() const { return SettingsPercentSizeInternal; }

	/** Returns the height of the scrollbox widget in percentages of the entire settings widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetScrollboxPercentHeight() const { return ScrollboxPercentHeightInternal; }

	/** Returns the padding of the settings widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FMargin& GetSettingsPadding() const { return SettingsPaddingInternal; }

	/** Returns the padding of the scrollbox widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FMargin& GetScrollboxPadding() const { return ScrollboxPaddingInternal; }

	/** Return the padding space, used on adding next column. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetSpaceBetweenColumns() const { return SpaceBetweenColumnsInternal; }

	/** Return the button theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FButtonThemeData& GetButtonThemeData() const { return ButtonThemeDataInternal; }

	/** Returns the checkbox theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCheckboxThemeData& GetCheckboxThemeData() const { return CheckboxThemeDataInternal; }

	/** Returns the combobox theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FComboboxThemeData& GetComboboxThemeData() const { return ComboboxThemeDataInternal; }

	/** Returns the slider theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FSliderThemeData& GetSliderThemeData() const { return SliderThemeDataInternal; }

	/** Returns the user input theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FSettingsThemeData& GetUserInputThemeData() const { return UserInputThemeDataInternal; }

	/** Returns the misc theme data. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FMiscThemeData& GetMiscThemeData() const { return MiscThemeDataInternal; }

protected:
	/** The data table with all settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Settings Data Table", ShowOnlyInnerProperties))
	TSoftObjectPtr<class USettingsDataTable> SettingsDataTableInternal; //[C]

	/** The sub-widget class of Button settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Button Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingButton> ButtonClassInternal; //[C]

	/** The sub-widget class of Checkbox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Checkbox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCheckbox> CheckboxClassInternal; //[C]

	/** The sub-widget class of Combobox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Combobox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCombobox> ComboboxClassInternal; //[C]

	/** The sub-widget class of Slider settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Slider Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingSlider> SliderClassInternal; //[C]

	/** The sub-widget class of Text Line settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Text Line Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingTextLine> TextLineClassInternal; //[C]

	/** The sub-widget class of User Input settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "User Input Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingUserInput> UserInputClassInternal; //[C]

	/** The width and height of the settings widget in percentages of an entire screen. Is clamped between 0 and 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Settings Percent Size", ClampMin = "0", ClampMax = "1", ShowOnlyInnerProperties))
	FVector2D SettingsPercentSizeInternal; //[C]

	/** The padding of the settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Settings Padding", ShowOnlyInnerProperties))
	FMargin SettingsPaddingInternal; //[C]

	/** The height of the scrollbox widget in percentages relatively to the content section, where 1 means fill all space under settings content section. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Scrollbox Percent Height", ClampMin = "0", ClampMax = "1", ShowOnlyInnerProperties))
	float ScrollboxPercentHeightInternal; //[C]

	/** The padding of the scrollbox widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Scrollbox Padding", ShowOnlyInnerProperties))
	FMargin ScrollboxPaddingInternal; //[C]

	/** The padding space, used on adding next column. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Space Between Columns", ShowOnlyInnerProperties))
	float SpaceBetweenColumnsInternal; //[C]

	/** The button theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Button Theme Data"))
	FButtonThemeData ButtonThemeDataInternal; //[C]

	/** The checkbox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Checkbox Theme Data"))
	FCheckboxThemeData CheckboxThemeDataInternal; //[C]

	/** The combobox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Combobox Theme Data"))
	FComboboxThemeData ComboboxThemeDataInternal; //[C]

	/** The slider theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Slider Theme Data"))
	FSliderThemeData SliderThemeDataInternal; //[C]

	/** The user input theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "User Input Theme Data"))
	FSettingsThemeData UserInputThemeDataInternal; //[C]

	/** The misc theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Misc Theme Data"))
	FMiscThemeData MiscThemeDataInternal; //[C]
};
