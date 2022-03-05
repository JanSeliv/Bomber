// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Structures/SettingsRow.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "SettingsWidget.generated.h"

/**
 * Describes common data of settings.
 */
UCLASS()
class USettingsDataAsset final : public UBomberDataAsset
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

protected:
	/** The data table with all settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Data Table"))
	TObjectPtr<class UDataTable> SettingsDataTableInternal = nullptr; //[D]

	/** The sub-widget class of Button settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Button Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingButton> ButtonClassInternal; //[D]

	/** The sub-widget class of Checkbox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Checkbox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCheckbox> CheckboxClassInternal; //[D]

	/** The sub-widget class of Combobox settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Combobox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCombobox> ComboboxClassInternal; //[D]

	/** The sub-widget class of Slider settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Slider Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingSlider> SliderClassInternal; //[D]

	/** The sub-widget class of Text Line settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Text Line Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingTextLine> TextLineClassInternal; //[D]

	/** The sub-widget class of User Input settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "User Input Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingUserInput> UserInputClassInternal; //[D]

	/** The width and height of the settings widget in percentages of an entire screen. Is clamped between 0 and 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Percent Size"))
	FVector2D SettingsPercentSizeInternal = FVector2D::UnitVector; //[D]

	/** The padding of the settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Padding"))
	FMargin SettingsPaddingInternal = 50.f; //[D]

	/** The height of the scrollbox widget in percentages of the entire settings widget, where 1 means fill all space under settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Scrollbox Percent Height", ClampMin = "0", ClampMax = "1"))
	float ScrollboxPercentHeightInternal = 0.7f; //[D]

	/** The padding of the scrollbox widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Scrollbox Padding"))
	FMargin ScrollboxPaddingInternal = 0.f; //[D]

	/** The padding space, used on adding next column. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Space Between Columns"))
	float SpaceBetweenColumnsInternal = 10.f; //[D]

	/** The button theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Button Theme Data"))
	FButtonThemeData ButtonThemeDataInternal; //[D]

	/** The checkbox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Checkbox Theme Data"))
	FCheckboxThemeData CheckboxThemeDataInternal; //[D]

	/** The combobox theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Combobox Theme Data"))
	FComboboxThemeData ComboboxThemeDataInternal; //[D]

	/** The slider theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Slider Theme Data"))
	FSliderThemeData SliderThemeDataInternal; //[D]

	/** The user input theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "User Input Theme Data"))
	FSettingsThemeData UserInputThemeDataInternal; //[D]

	/** The misc theme data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Misc Theme Data"))
	FMiscThemeData MiscThemeDataInternal; //[D]
};

/**
 * The UI widget of settings.
 * It generates and manages settings specified in rows of the Settings Data Table.
 */
UCLASS()
class USettingsWidget final : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Display settings on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OpenSettings();

	/** Save and close the settings widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void CloseSettings();

	/** Flip-flop opens and closes the Settings menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ToggleSettings();

	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Try to find the setting row.
	* @param PotentialTagName The probable tag name by which the row will be found (for 'VSync' will find a row with 'Settings.Checkbox.VSync' tag).
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FSettingsPicker& FindSettingRow(FName PotentialTagName) const;

	/** Returns the found row by specified tag.
	* @param SettingTag The gameplay tag by which the row will be found.
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "SettingTag"))
	const FSettingsPicker& GetSettingRow(const FGameplayTag& SettingTag) const;

	/** Save all settings into their configs. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SaveSettings();

	/** Update settings on UI.
	 * @param SettingsToUpdate Contains tags of settings that are needed to update. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateSettings(const FGameplayTagContainer& SettingsToUpdate);

	/** Returns the name of found tag by specified function. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FGameplayTag& GetTagByFunctionPicker(const FFunctionPicker& FunctionPicker) const;

	/* ---------------------------------------------------
	 *		Setters by setting types
	 * --------------------------------------------------- */

	/**
   	  * Set value to the option by tag.
   	  * Common function to set setting of an any type by the string.
   	  * Used by cheat manager to override any setting.
	  *	@param TagName The key by which the row will be find.
	  * @param Value The value in a string format.
	  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Value"))
	void SetSettingValue(FName TagName, const FString& Value);

	/** Press button. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "ButtonTag"))
	void SetSettingButtonPressed(const FGameplayTag& ButtonTag);

	/** Toggle checkbox. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CheckboxTag"))
	void SetSettingCheckbox(const FGameplayTag& CheckboxTag, bool InValue);

	/** Set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void SetSettingComboboxIndex(const FGameplayTag& ComboboxTag, int32 InValue);

	/** Set new members for a combobox. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "ComboboxTag,InValue"))
	void SetSettingComboboxMembers(const FGameplayTag& ComboboxTag, const TArray<FText>& InValue);

	/** Set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "SliderTag"))
	void SetSettingSlider(const FGameplayTag& SliderTag, float InValue);

	/** Set new text. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "TextLineTag,InValue"))
	void SetSettingTextLine(const FGameplayTag& TextLineTag, const FText& InValue);

	/** Set new text for an input box. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "UserInputTag"))
	void SetSettingUserInput(const FGameplayTag& UserInputTag, FName InValue);

	/** Set new custom widget for setting by specified tag. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	void SetSettingCustomWidget(const FGameplayTag& CustomWidgetTag, class USettingCustomWidget* InCustomWidget);

	/* ---------------------------------------------------
	 *		Getters by setting types
	 * --------------------------------------------------- */

	/** Returns is a checkbox toggled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CheckboxTag"))
	bool GetCheckboxValue(const FGameplayTag& CheckboxTag) const;

	/** Returns chosen member index of a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ComboboxTag"))
	int32 GetComboboxIndex(const FGameplayTag& ComboboxTag) const;

	/** Get all members of a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void GetComboboxMembers(const FGameplayTag& ComboboxTag, TArray<FText>& OutMembers) const;

	/** Get current value of a slider [0...1]. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "SliderTag"))
	float GetSliderValue(const FGameplayTag& SliderTag) const;

	/** Get current text of the text line setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "TextLineTag"))
	void GetTextLineValue(const FGameplayTag& TextLineTag, FText& OutText) const;

	/** Get current input name of the text input setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "UserInputTag"))
	FName GetUserInputValue(const FGameplayTag& UserInputTag) const;

	/** Get custom widget of the setting by specified tag.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	class USettingCustomWidget* GetCustomWidget(const FGameplayTag& CustomWidgetTag) const;

	/** Get setting widget object by specified tag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (AutoCreateRefTerm = "SettingTag"))
	class USettingSubWidget* GetSettingSubWidget(const FGameplayTag& SettingTag) const;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Contains all settings. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Table Rows"))
	TMap<FName/*Tag*/, FSettingsPicker/*Row*/> SettingsTableRowsInternal; //[G]

	/** The index of the current column. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Column Index"))
	int32 CurrentColumnIndexInternal; //[G]

	/** Is set automatically on started by amount of rows that are marked to be started on next column. Settings have at least one column. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Overall Columns Num"))
	int32 OverallColumnsNumInternal = 1; //[G]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Construct all settings from the settings data table. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void ConstructSettings();

	/** Is called when all game widgets are initialized to construct settings. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnWidgetsInitialized();

	/** Is called when visibility is changed for this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnVisibilityChange(ESlateVisibility InVisibility);

	/** Bind and set static object delegate.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryBindStaticContext(UPARAM(ref)FSettingsPrimary& Primary);

	/** Creates setting sub-widget (like button, checkbox etc.) based on specified setting class and sets it to specified primary data.
	* @param Primary The Data that should contain created setting class.
	* @param SettingSubWidgetClass The setting widget class to create. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "Primary,SettingSubWidgetClass"))
	void CreateSettingSubWidget(FSettingsPrimary& Primary, const TSubclassOf<USettingSubWidget>& SettingSubWidgetClass);

	/** Starts adding settings on the next column. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void StartNextColumn();

	/* ---------------------------------------------------
	 *		Add by setting types
	 * --------------------------------------------------- */

	/** Add setting on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void AddSetting(UPARAM(ref)FSettingsPicker& Setting);

	/** Add button on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddButton(const FSettingsPrimary& Primary, const FSettingsButton& Data);
	void AddSettingButton(FSettingsPrimary& Primary, FSettingsButton& Data);

	/** Add checkbox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCheckbox(const FSettingsPrimary& Primary, const FSettingsCheckbox& Data);
	void AddSettingCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data);

	/** Add combobox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCombobox(const FSettingsPrimary& Primary, const FSettingsCombobox& Data);
	void AddSettingCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data);

	/** Add slider on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddSlider(const FSettingsPrimary& Primary, const FSettingsSlider& Data);
	void AddSettingSlider(FSettingsPrimary& Primary, FSettingsSlider& Data);

	/** Add simple text on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddTextLine(const FSettingsPrimary& Primary, const FSettingsTextLine& Data);
	void AddSettingTextLine(FSettingsPrimary& Primary, FSettingsTextLine& Data);

	/** Add text input on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddUserInput(const FSettingsPrimary& Primary, const FSettingsUserInput& Data);
	void AddSettingUserInput(FSettingsPrimary& Primary, FSettingsUserInput& Data);

	/** Add custom widget on UI.  */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCustomWidget(const FSettingsPrimary& Primary, const FSettingsCustomWidget& Data);
	void AddSettingCustomWidget(FSettingsPrimary& Primary, FSettingsCustomWidget& Data);

	/* ---------------------------------------------------
	 *		Blueprint implementable setters
	 * --------------------------------------------------- */

	/** Internal blueprint function to toggle checkbox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "CheckboxTag"))
	void SetCheckbox(const FGameplayTag& CheckboxTag, bool InValue);

	/** Internal blueprint function to set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ComboboxTag"))
	void SetComboboxIndex(const FGameplayTag& ComboboxTag, int32 InValue);

	/** Internal blueprint function to set new members for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ComboboxTag,InValue"))
	void SetComboboxMembers(const FGameplayTag& ComboboxTag, const TArray<FText>& InValue);

	/** Internal blueprint function to set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "SliderTag"))
	void SetSlider(const FGameplayTag& SliderTag, float InValue);

	/** Internal blueprint function to set new text for an input box. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "UserInputTag"))
	void SetUserInput(const FGameplayTag& UserInputTag, FName InValue);
};
