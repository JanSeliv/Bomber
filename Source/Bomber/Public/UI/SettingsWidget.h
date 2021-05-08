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

	/** Returns the width and height of the settings widget in percentages of an entire screen. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FVector2D GetSettingsPercentSize() const { return SettingsPercentSizeInternal; }

	/** Returns the height of the scrollbox widget in percentages of the entire settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetScrollboxPercentHeight() const { return ScrollboxPercentHeightInternal; }

	/** Returns the padding of the settings widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FMargin GetSettingsPadding() const { return SettingsPaddingInternal; }

	/** Returns the padding of the scrollbox widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FMargin GetScrollboxPadding() const { return ScrollboxPaddingInternal; }

	/** Return the padding space, used on adding next column. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetSpaceBetweenColumns() const { return SpaceBetweenColumnsInternal; }

protected:
	/** The data table with all settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Data Table", ShowOnlyInnerProperties))
	class UDataTable* SettingsDataTableInternal; //[D]

	/** The width and height of the settings widget in percentages of an entire screen. Is clamped between 0 and 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Percent Size", ShowOnlyInnerProperties))
	FVector2D SettingsPercentSizeInternal = FVector2D::UnitVector; //[D]

	/** The padding of the settings widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Padding", ShowOnlyInnerProperties))
	FMargin SettingsPaddingInternal = 50.f; //[D]

	/** The height of the scrollbox widget in percentages of the entire settings widget, where 1 means fill all space under settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Scrollbox Percent Height", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "1"))
	float ScrollboxPercentHeightInternal = 0.7f; //[D]

	/** The padding of the scrollbox widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Scrollbox Padding", ShowOnlyInnerProperties))
	FMargin ScrollboxPaddingInternal = 0.f; //[D]

	/** The padding space, used on adding next column. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Space Between Columns", ShowOnlyInnerProperties))
	float SpaceBetweenColumnsInternal = 10.f; //[D]
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
	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Returns the found row by specified tag.
	* @param TagName The key by which the row will be find.
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FSettingsPicker FindSettingRow(FName TagName) const;

	/** Save all settings into their configs. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SaveSettings();

	/* ---------------------------------------------------
	 *		Setters by setting types
	 * --------------------------------------------------- */

	/** Set value to the option by tag. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSettingValue(FName TagName, const FString& Value);

	/** Press button. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetButtonPressed(FName TagName);

	/** Toggle checkbox. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetCheckbox(FName TagName, bool InValue);

	/** Set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetComboboxIndex(FName TagName, int32 InValue);

	/** Set new members for a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetComboboxMembers(FName TagName, const TArray<FText>& InValue);

	/** Set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetSlider(FName TagName, float InValue);

	/** Set new text. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetTextSimple(FName TagName, const FText& InValue);

	/** Set new text for an input box. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void SetTextInput(FName TagName, const FText& InValue);

	/* ---------------------------------------------------
	 *		Getters by setting types
	 * --------------------------------------------------- */

	/** Returns is a checkbox toggled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	bool GetCheckboxValue(FName TagName) const;

	/** Returns chosen member index of a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	int32 GetComboboxIndex(FName TagName) const;

	/** Get all members of a combobox. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	TArray<FText> GetComboboxMembers(FName TagName) const;

	/** Get current value of a slider [0...1]. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	float GetSliderValue(FName TagName) const;

	/** Get current text of a simple or input text widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FText GetTextValue(FName TagName) const;

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

	/** Called when the visibility has changed.
	 * @param InVisibility New visibility. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnVisibilityChange(ESlateVisibility InVisibility);

	/** Bind and set static object delegate.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryBindStaticContext(UPARAM(ref)FSettingsPrimary& Primary);

	/** Bind on text getter and setter.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryBindTextFunctions(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsTextSimple& Data);

	/** Save and close the settings widget. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CloseSettings();

	/* ---------------------------------------------------
	 *		Add by setting types
	 * --------------------------------------------------- */

	/** Add setting on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void AddSetting(UPARAM(ref)FSettingsPicker& Setting);

	/** Add button on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddButton", AutoCreateRefTerm = "Primary,Data"))
	void AddButtonBP(const FSettingsPrimary& Primary, const FSettingsButton& Data);
	void AddButton(FSettingsPrimary& Primary, FSettingsButton& Data);

	/** Add checkbox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddCheckbox", AutoCreateRefTerm = "Primary,Data"))
	void AddCheckboxBP(const FSettingsPrimary& Primary, const FSettingsCheckbox& Data);
	void AddCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data);

	/** Add combobox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddCombobox", AutoCreateRefTerm = "Primary,Data"))
	void AddComboboxBP(const FSettingsPrimary& Primary, const FSettingsCombobox& Data);
	void AddCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data);

	/** Add slider on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddSlider", AutoCreateRefTerm = "Primary,Data"))
	void AddSliderBP(const FSettingsPrimary& Primary, const FSettingsSlider& Data);
	void AddSlider(FSettingsPrimary& Primary, FSettingsSlider& Data);

	/** Add simple text on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddTextSimple", AutoCreateRefTerm = "Primary,Data"))
	void AddTextSimpleBP(const FSettingsPrimary& Primary, const FSettingsTextSimple& Data);
	void AddTextSimple(FSettingsPrimary& Primary, FSettingsTextSimple& Data);

	/** Add text input on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddTextInput", AutoCreateRefTerm = "Primary,Data"))
	void AddTextInputBP(const FSettingsPrimary& Primary, const FSettingsTextInput& Data);
	void AddTextInput(FSettingsPrimary& Primary, FSettingsTextInput& Data);

	/** Starts adding settings on the next column. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void StartNextColumn();
};
