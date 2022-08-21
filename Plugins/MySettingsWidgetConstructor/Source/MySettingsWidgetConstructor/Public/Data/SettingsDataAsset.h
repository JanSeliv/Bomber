// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataAsset.h"
//---
#include "Structures/SettingsRow.h"
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
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USettingsDataTable* GetSettingsDataTable() const { return SettingsDataTableInternal; }

	/** Returns the sub-widget of Button settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingButton> GetButtonClass() const { return ButtonClassInternal; }

	/** Returns the sub-widget of Checkbox settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingCheckbox> GetCheckboxClass() const { return CheckboxClassInternal; }

	/** Returns the sub-widget of Combobox settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingCombobox> GetComboboxClass() const { return ComboboxClassInternal; }

	/** Returns the sub-widget of Slider settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingSlider> GetSliderClass() const { return SliderClassInternal; }

	/** Returns the sub-widget of Text Line settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingTextLine> GetTextLineClass() const { return TextLineClassInternal; }

	/** Returns the sub-widget of User Input settings. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingUserInput> GetUserInputClass() const { return UserInputClassInternal; }

	/** Returns the width and height of the settings widget in percentages of an entire screen. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FVector2D& GetSettingsPercentSize() const { return SettingsPercentSizeInternal; }

	/** Returns the height of the scrollbox widget in percentages of the entire settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetScrollboxPercentHeight() const { return ScrollboxPercentHeightInternal; }

	/** Returns the padding of the settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FMargin& GetSettingsPadding() const { return SettingsPaddingInternal; }

	/** Returns the padding of the scrollbox widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FMargin& GetScrollboxPadding() const { return ScrollboxPaddingInternal; }

	/** Return the padding space, used on adding next column. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetSpaceBetweenColumns() const { return SpaceBetweenColumnsInternal; }

	/** Return the button theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FButtonThemeData& GetButtonThemeData() const { return ButtonThemeDataInternal; }

	/** Returns the checkbox theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FCheckboxThemeData& GetCheckboxThemeData() const { return CheckboxThemeDataInternal; }

	/** Returns the combobox theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FComboboxThemeData& GetComboboxThemeData() const { return ComboboxThemeDataInternal; }

	/** Returns the slider theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FSliderThemeData& GetSliderThemeData() const { return SliderThemeDataInternal; }

	/** Returns the user input theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FSettingsThemeData& GetUserInputThemeData() const { return UserInputThemeDataInternal; }

	/** Returns the misc theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FMiscThemeData& GetMiscThemeData() const { return MiscThemeDataInternal; }

	/** Returns the misc theme data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool ShouldAutoConstructSettings() const { return bAutoConstructSettingsInternal; }

protected:
	/** The data table with all settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Data Table"))
	TObjectPtr<class USettingsDataTable> SettingsDataTableInternal = nullptr; //[D]

	/** The sub-widget class of Button settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Button Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingButton> ButtonClassInternal = nullptr; //[D]

	/** The sub-widget class of Checkbox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Checkbox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCheckbox> CheckboxClassInternal = nullptr; //[D]

	/** The sub-widget class of Combobox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Combobox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCombobox> ComboboxClassInternal = nullptr; //[D]

	/** The sub-widget class of Slider settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Slider Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingSlider> SliderClassInternal = nullptr; //[D]

	/** The sub-widget class of Text Line settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Text Line Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingTextLine> TextLineClassInternal = nullptr; //[D]

	/** The sub-widget class of User Input settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "User Input Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingUserInput> UserInputClassInternal = nullptr; //[D]

	/** Set true to automatic construct settings during USettingsWidget::NativeConstruct() . */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Auto Construct Settings", ShowOnlyInnerProperties))
	bool bAutoConstructSettingsInternal = true; //[D]

	/** The width and height of the settings widget in percentages of an entire screen. Is clamped between 0 and 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Settings Percent Size", ShowOnlyInnerProperties))
	FVector2D SettingsPercentSizeInternal = FVector2D(0.95f, 0.8f); //[D]

	/** The padding of the settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Settings Padding", ShowOnlyInnerProperties))
	FMargin SettingsPaddingInternal = 50.f; //[D]

	/** The height of the scrollbox widget in percentages of the entire settings widget, where 1 means fill all space under settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Scrollbox Percent Height", ClampMin = "0", ClampMax = "1", ShowOnlyInnerProperties))
	float ScrollboxPercentHeightInternal = 0.75f; //[D]

	/** The padding of the scrollbox widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Scrollbox Padding", ShowOnlyInnerProperties))
	FMargin ScrollboxPaddingInternal = 0.f; //[D]

	/** The padding space, used on adding next column. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Space Between Columns", ShowOnlyInnerProperties))
	float SpaceBetweenColumnsInternal = 10.f; //[D]

	/** The button theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Button Theme Data"))
	FButtonThemeData ButtonThemeDataInternal; //[D]

	/** The checkbox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Checkbox Theme Data"))
	FCheckboxThemeData CheckboxThemeDataInternal; //[D]

	/** The combobox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Combobox Theme Data"))
	FComboboxThemeData ComboboxThemeDataInternal; //[D]

	/** The slider theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Slider Theme Data"))
	FSliderThemeData SliderThemeDataInternal; //[D]

	/** The user input theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "User Input Theme Data"))
	FSettingsThemeData UserInputThemeDataInternal; //[D]

	/** The misc theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, meta = (BlueprintProtected, DisplayName = "Misc Theme Data"))
	FMiscThemeData MiscThemeDataInternal; //[D]
};
