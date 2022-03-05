﻿// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "GameplayTagContainer.h"
#include "Widgets/Input/SComboBox.h"
//---
#include "SettingSubWidget.generated.h"

typedef SComboBox<TSharedPtr<FString>> SComboboxString;

/**
 * The base class of specific setting like button, checkbox, combobox, slider, text line, user input.
 */
UCLASS()
class USettingSubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the slate widget from UMG widget.
	 * For example: it will return SCheckbox from UCheckBox. */
	template <typename T>
	TSharedPtr<T> GetSlateWidget(const UWidget* ForWidget) const { return ForWidget ? StaticCastSharedPtr<T>(ForWidget->GetCachedWidget()) : nullptr; }

	/** Returns the widget that shows the caption text of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UTextBlock* GetCaptionWidget() const { return CaptionWidget; }

	/** Returns the Size Box widget . */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USizeBox* GetSizeBoxWidget() const { return SizeBoxWidget; }

	/** Returns the custom line height for this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	float GetLineHeight() const;

	/** Set custom line height for this setting. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetLineHeight(float NewLineHeight);

	/** Returns the caption text that is shown on UI. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetCaptionText(FText& OutCaptionText) const;

	/** Set the new caption text on UI for this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCaptionText(const FText& NewCaptionText);

	/** Returns the setting tag of this widget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FGameplayTag& GetSettingTag() const { return SettingTagInternal; }

	/** Set the new setting tag for this widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetSettingTag(const FGameplayTag& NewSettingTag);

protected:
	/** The Size Box widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USizeBox> SizeBoxWidget = nullptr; //[I]

	/** The widget that shows the caption text of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> CaptionWidget = nullptr; //[I]

	/** The setting tag of this widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Setting Tag"))
	FGameplayTag SettingTagInternal; //[G]

	/** The main settings widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<class USettingsWidget> SettingsWidgetInternal; //[G]

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;
};

/**
 * The sub-widget of Button settings.
 */
UCLASS()
class USettingButton final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual button widget of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UButton* GetButtonWidget() const { return ButtonWidget; }

	/** Returns the slate button. */
	FORCEINLINE TSharedPtr<class SButton> GetSlateButton() const { return SlateButtonInternal.Pin(); }

protected:
	/** The slate button.*/
	TWeakPtr<class SButton> SlateButtonInternal = nullptr;

	/** The actual button widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> ButtonWidget = nullptr; //[I]

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the Button Widget is pressed.
	 * @see USettingButton::OnSettingButtonPressed */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnButtonPressed();
};

/**
 * The sub-widget of Checkbox settings.
 */
UCLASS()
class USettingCheckbox final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual checkbox widget of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UCheckBox* GetCheckboxWidget() const { return CheckboxWidget; }

	/** Returns the slate checkbox. */
	FORCEINLINE TSharedPtr<class SCheckBox> GetSlateCheckbox() const { return SlateCheckboxInternal.Pin(); }

protected:
	/** The slate checkbox.*/
	TWeakPtr<class SCheckBox> SlateCheckboxInternal = nullptr;

	/** The actual checkbox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UCheckBox> CheckboxWidget = nullptr; //[I]

	/** Called after the underlying slate widget is constructed.
 	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the checked state has changed.
	 * @see USettingCheckbox::CheckboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCheckStateChanged(bool bIsChecked);
};

/**
 * The sub-widget of Combobox settings.
 */
UCLASS()
class USettingCombobox final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual combobox widget of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UComboBoxString* GetComboboxWidget() const { return ComboboxWidget; }

	/** Returns the slate combobox. */
	FORCEINLINE TSharedPtr<SComboboxString> GetSlateCombobox() const { return SlateComboboxInternal.Pin(); }

	/** Returns true if combobox is opened. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsComboboxOpened() const { return bIsComboboxOpenedInternal; }

protected:
	/** The slate combobox.*/
	TWeakPtr<SComboboxString> SlateComboboxInternal = nullptr;

	/** The actual combobox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UComboBoxString> ComboboxWidget = nullptr; //[I]

	/** Is true if combobox is currently opened in Settings. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Combobox Opened"))
	bool bIsComboboxOpenedInternal; //[G]

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is executed every tick when widget is enabled. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Called when a new item is selected in the combobox
	 * @see USettingCheckbox::ComboboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** Called when the combobox is opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMenuOpenChanged();
};

/**
 * The sub-widget of Slider settings.
 */
UCLASS()
class USettingSlider final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual slider widget of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USlider* GetSliderWidget() const { return SliderWidget; }

	/** Returns the slate slider. */
	FORCEINLINE TSharedPtr<class SSlider> GetSlateSlider() const { return SlateSliderInternal.Pin(); }

protected:
	/** The slate slider.*/
	TWeakPtr<class SSlider> SlateSliderInternal = nullptr;

	/** The actual slider widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USlider> SliderWidget = nullptr; //[I]

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Invoked when the mouse is released and a capture ends. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMouseCaptureEnd();

	/** Called when the value is changed by slider or typing.
	 * @see USettingCheckbox::SliderWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnValueChanged(float Value);
};

/**
 * The sub-widget of Text Line settings.
 */
UCLASS()
class USettingTextLine final : public USettingSubWidget
{
	GENERATED_BODY()

public:
};

/**
 * The sub-widget of User Input settings.
 */
UCLASS()
class USettingUserInput final : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual Editable Text Box widget of this setting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UEditableTextBox* GetEditableTextBox() const { return EditableTextBox; }

	/** Returns current text set in the Editable Text Box. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	void GetEditableText(FText& OutText) const;

	/** Set new text programmatically instead of by the user. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetEditableText(const FText& InText);

	/** Returns the slate editable text box. */
	FORCEINLINE TSharedPtr<class SEditableTextBox> GetSlateEditableTextBox() const { return SlateEditableTextBoxInternal.Pin(); }

protected:
	/** The slate editable text box.*/
	TWeakPtr<class SEditableTextBox> SlateEditableTextBoxInternal = nullptr;

	/** The actual Editable Text Box widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UEditableTextBox> EditableTextBox = nullptr; //[I]

	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called whenever the text is changed programmatically or interactively by the user.
	 * @see USettingCheckbox::EditableTextBoxInternal */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnTextChanged(const FText& Text);
};

/**
 * The sub-widget of the custom widget settings.
 */
UCLASS()
class USettingCustomWidget : public USettingSubWidget
{
	GENERATED_BODY()

public:
};
