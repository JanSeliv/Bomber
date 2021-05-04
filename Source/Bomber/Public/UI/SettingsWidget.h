// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Structures/SettingsRow.h"
//---
#include "SettingsWidget.generated.h"

/**
 * The UI widget of settings.
 */
UCLASS(Abstract)
class BOMBER_API USettingsWidget final : public UUserWidget
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
	TMap<FName/*Tag*/, FSettingsPicker> SettingsTableRowsInternal; //[D]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/**  */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ConstructSettings();

	/** Bind and set static object delegate.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryBindStaticContext(UPARAM(ref)FSettingsPrimary& Primary);

	/** Bind on text getter and setter.
	* @see FSettingsPrimary::OnStaticContext */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryBindTextFunctions(FSettingsPrimary& Primary, UPARAM(ref)FSettingsTextSimple& Data);

	/* ---------------------------------------------------
	 *		Add by setting types
	 * --------------------------------------------------- */

	/** Add setting on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void AddSetting(UPARAM(ref)FSettingsPicker& Setting);

	/** Add button on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddButton"))
	void AddButtonBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsButton& Data);
	void AddButton(FSettingsPrimary& Primary, FSettingsButton& Data);

	/** Add checkbox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddCheckbox"))
	void AddCheckboxBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsCheckbox& Data);
	void AddCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data);

	/** Add combobox on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddCombobox"))
	void AddComboboxBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsCombobox& Data);
	void AddCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data);

	/** Add slider on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddSlider"))
	void AddSliderBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsSlider& Data);
	void AddSlider(FSettingsPrimary& Primary, FSettingsSlider& Data);

	/** Add simple text on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddTextSimple"))
	void AddTextSimpleBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsTextSimple& Data);
	void AddTextSimple(FSettingsPrimary& Primary, FSettingsTextSimple& Data);

	/** Add text input on UI. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "C++", meta = (BlueprintProtected, OverrideNativeName = "AddTextInput"))
	void AddTextInputBP(UPARAM(ref)FSettingsPrimary& Primary, UPARAM(ref)FSettingsTextInput& Data);
	void AddTextInput(FSettingsPrimary& Primary, FSettingsTextInput& Data);
};
