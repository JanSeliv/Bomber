// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Data/SettingsRow.h"
//---
#include "SettingsWidget.generated.h"

/**
 * The UI widget of settings.
 * It generates and manages settings specified in rows of the Settings Data Table.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggledSettings, bool, bIsVisible);

	/** Is called to notify listeners the Settings widget is opened or closed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Settings Widget Constructor")
	FOnToggledSettings OnToggledSettings;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Display settings on UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void OpenSettings();

	/** Is called on displayed settings on UI. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void OnOpenSettings();

	/** Save and close the settings widget. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void CloseSettings();

	/** Is called on closed settings on UI. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void OnCloseSettings();

	/** Flip-flop opens and closes the Settings menu. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void ToggleSettings();

	/** Is called to player sound effect on any setting click. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void PlayUIClickSFX();

	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Try to find the setting row.
	* @param PotentialTagName The probable tag name by which the row will be found (for 'VSync' will find a row with 'Settings.Checkbox.VSync' tag).
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FSettingsPicker& FindSettingRow(FName PotentialTagName) const;

	/** Returns the found row by specified tag.
	* @param SettingTag The gameplay tag by which the row will be found. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingTag"))
	const FSettingsPicker& GetSettingRow(const FSettingTag& SettingTag) const;

	/** Save all settings into their configs. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void SaveSettings();

	/** Apply all current settings on device. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void ApplySettings();

	/** Update settings on UI.
	 * @param SettingsToUpdate Contains tags of settings that are needed to update. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingsToUpdate"))
	void UpdateSettings(
		UPARAM(meta = (Categories = "Settings")) const FGameplayTagContainer& SettingsToUpdate);

	/** Returns the name of found tag by specified function. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingFunction"))
	const FSettingTag& GetTagByFunction(const FSettingFunctionPicker& SettingFunction) const;

	/* ---------------------------------------------------
	 *		Style
	 * --------------------------------------------------- */

	/** Returns the size of the Settings widget on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FVector2D GetSettingsSize() const;

	/** Returns the size of specified sections on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style")
	FVector2D GetSubWidgetsSize(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/SettingsWidgetConstructor.EMyVerticalAlignment")) int32 SectionsBitmask) const;

	/** Returns the height of a setting scrollbox on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style")
	float GetScrollBoxHeight() const;

	/** Is blueprint-event called that returns the style brush by specified button state. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style", meta = (BlueprintProtected))
	FSlateBrush GetButtonBrush(ESettingsButtonState State) const;

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
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "Value"))
	void SetSettingValue(FName TagName, const FString& Value);

	/** Press button. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "ButtonTag"))
	void SetSettingButtonPressed(const FSettingTag& ButtonTag);

	/** Toggle checkbox. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "CheckboxTag"))
	void SetSettingCheckbox(const FSettingTag& CheckboxTag, bool InValue);

	/** Set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void SetSettingComboboxIndex(const FSettingTag& ComboboxTag, int32 InValue);

	/** Set new members for a combobox. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "ComboboxTag,InValue"))
	void SetSettingComboboxMembers(const FSettingTag& ComboboxTag, const TArray<FText>& InValue);

	/** Set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "SliderTag"))
	void SetSettingSlider(const FSettingTag& SliderTag, double InValue);

	/** Set new text. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "TextLineTag,InValue"))
	void SetSettingTextLine(const FSettingTag& TextLineTag, const FText& InValue);

	/** Set new text for an input box. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "UserInputTag"))
	void SetSettingUserInput(const FSettingTag& UserInputTag, FName InValue);

	/** Set new custom widget for setting by specified tag. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	void SetSettingCustomWidget(const FSettingTag& CustomWidgetTag, class USettingCustomWidget* InCustomWidget);

	/** Creates setting sub-widget (like button, checkbox etc.) based on specified setting class and sets it to specified primary data.
	 * @param InOutPrimary The Data that should contain created setting class.
	 * @param SettingSubWidgetClass The setting widget class to create. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected, AutoCreateRefTerm = "InOutPrimary"))
	USettingSubWidget* CreateSettingSubWidget(UPARAM(ref) FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass);

	/** Creates setting sub-widget (like button, checkbox etc.) based on specified setting class and sets it to specified primary data. */
	template <typename T = USettingSubWidget>
	FORCEINLINE T* CreateSettingSubWidget(FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass) { return Cast<T>(CreateSettingSubWidget(InOutPrimary, SettingSubWidgetClass)); }

	/* ---------------------------------------------------
	 *		Getters by setting types
	 * --------------------------------------------------- */

	/** Returns is a checkbox toggled. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "CheckboxTag"))
	bool GetCheckboxValue(const FSettingTag& CheckboxTag) const;

	/** Returns chosen member index of a combobox. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	int32 GetComboboxIndex(const FSettingTag& ComboboxTag) const;

	/** Get all members of a combobox. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void GetComboboxMembers(const FSettingTag& ComboboxTag, TArray<FText>& OutMembers) const;

	/** Get current value of a slider [0...1]. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "SliderTag"))
	double GetSliderValue(const FSettingTag& SliderTag) const;

	/** Get current text of the text line setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "TextLineTag"))
	void GetTextLineValue(const FSettingTag& TextLineTag, FText& OutText) const;

	/** Get current input name of the text input setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "UserInputTag"))
	FName GetUserInputValue(const FSettingTag& UserInputTag) const;

	/** Get custom widget of the setting by specified tag.  */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	class USettingCustomWidget* GetCustomWidget(const FSettingTag& CustomWidgetTag) const;

	/** Get setting widget object by specified tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "SettingTag"))
	class USettingSubWidget* GetSettingSubWidget(const FSettingTag& SettingTag) const;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Contains all settings. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Table Rows"))
	TMap<FName/*Tag*/, FSettingsPicker/*Row*/> SettingsTableRowsInternal;

	/** The index of the current column. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Transient, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Current Column Index"))
	int32 CurrentColumnIndexInternal = 0;

	/** Is set automatically on started by amount of rows that are marked to be started on next column. Settings have at least one column. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Overall Columns Num"))
	int32 OverallColumnsNumInternal = 1;

	/** Contains all setting scrollboxes added to columns. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Setting ScrollBoxes"))
	TArray<TObjectPtr<class USettingScrollBox>> SettingScrollBoxesInternal;

	/* ---------------------------------------------------
	 *		Bound widget properties
	 * --------------------------------------------------- */

	/** The section in the top margin of Settings, usually contains a title. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> HeaderVerticalBox = nullptr;

	/** The main section in the middle of Settings, contains all in-game settings. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UHorizontalBox> ContentHorizontalBox = nullptr;

	/** The section in the bottom margin of Settings, usually contains the Back button*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> FooterVerticalBox = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Constructs settings if viewport is ready otherwise Wait until viewport become initialized. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void TryConstructSettings();

	/** Is called right after the game was started and windows size is set to construct settings. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);

	/** Is blueprint-event called on settings construct to cache some data before creating subwidgets. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void OnConstructSettings();
	void ConstructSettings();

	/** Internal function to cache setting rows from Settings Data Table. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void UpdateSettingsTableRows();

	/** Is called when In-Game menu became opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void OnToggleSettings(bool bIsVisible);

	/** Bind and set static object delegate.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void TryBindStaticContext(UPARAM(ref)FSettingsPrimary& Primary);

	/** Starts adding settings on the next column. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void StartNextColumn();

	/** Automatically sets the height for all scrollboxes in the Settings. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void UpdateScrollBoxesHeight();

	/* ---------------------------------------------------
	 *		Add by setting types
	 * --------------------------------------------------- */

	/** Add setting on UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected))
	void AddSetting(UPARAM(ref)FSettingsPicker& Setting);

	/** Add button on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddButton(const FSettingsPrimary& Primary, const FSettingsButton& Data);
	void AddSettingButton(FSettingsPrimary& Primary, FSettingsButton& Data);

	/** Add checkbox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCheckbox(const FSettingsPrimary& Primary, const FSettingsCheckbox& Data);
	void AddSettingCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data);

	/** Add combobox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCombobox(const FSettingsPrimary& Primary, const FSettingsCombobox& Data);
	void AddSettingCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data);

	/** Add slider on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddSlider(const FSettingsPrimary& Primary, const FSettingsSlider& Data);
	void AddSettingSlider(FSettingsPrimary& Primary, FSettingsSlider& Data);

	/** Add simple text on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddTextLine(const FSettingsPrimary& Primary, const FSettingsTextLine& Data);
	void AddSettingTextLine(FSettingsPrimary& Primary, FSettingsTextLine& Data);

	/** Add text input on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddUserInput(const FSettingsPrimary& Primary, const FSettingsUserInput& Data);
	void AddSettingUserInput(FSettingsPrimary& Primary, FSettingsUserInput& Data);

	/** Add custom widget on UI.  */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void AddCustomWidget(const FSettingsPrimary& Primary, const FSettingsCustomWidget& Data);
	void AddSettingCustomWidget(FSettingsPrimary& Primary, FSettingsCustomWidget& Data);

	/* ---------------------------------------------------
	 *		Blueprint implementable setters
	 * --------------------------------------------------- */

	/** Internal blueprint function to toggle checkbox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Setters", meta = (BlueprintProtected, AutoCreateRefTerm = "CheckboxTag"))
	void SetCheckbox(const FSettingTag& CheckboxTag, bool InValue);

	/** Internal blueprint function to set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Setters", meta = (BlueprintProtected, AutoCreateRefTerm = "ComboboxTag"))
	void SetComboboxIndex(const FSettingTag& ComboboxTag, int32 InValue);

	/** Internal blueprint function to set new members for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Setters", meta = (BlueprintProtected, AutoCreateRefTerm = "ComboboxTag,InValue"))
	void SetComboboxMembers(const FSettingTag& ComboboxTag, const TArray<FText>& InValue);

	/** Internal blueprint function to set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Setters", meta = (BlueprintProtected, AutoCreateRefTerm = "SliderTag"))
	void SetSlider(const FSettingTag& SliderTag, float InValue);

	/** Internal blueprint function to set new text for an input box. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor|Setters", meta = (BlueprintProtected, AutoCreateRefTerm = "UserInputTag"))
	void SetUserInput(const FSettingTag& UserInputTag, FName InValue);
};
