// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Data/SettingsRow.h"
#include "Widgets/Input/SComboBox.h"
//---
#include "SettingSubWidget.generated.h"

/**
 * The base class of specific setting like button, checkbox, combobox, slider, text line, user input etc.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingSubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the widget that shows the caption text of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UTextBlock* GetCaptionWidget() const { return CaptionWidget; }

	/** Returns the Size Box widget . */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class USizeBox* GetSizeBoxWidget() const { return SizeBoxWidget; }

	/** Returns the custom line height for this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	float GetLineHeight() const;

	/** Set custom line height for this setting. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget")
	void SetLineHeight(float NewLineHeight);

	/** Returns the caption text that is shown on UI. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	void GetCaptionText(FText& OutCaptionText) const;

	/** Set the new caption text on UI for this widget. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (AutoCreateRefTerm = "NewCaptionText"))
	void SetCaptionText(const FText& NewCaptionText);

	/** Returns the setting tag of this widget. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	const FORCEINLINE FSettingTag& GetSettingTag() const { return SettingPrimaryRowInternal.Tag; }

	/** Returns the setting primary row of this widget. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	const FORCEINLINE FSettingsPrimary& GetSettingPrimaryRow() const { return SettingPrimaryRowInternal; }

	/** Set the new setting tag for this widget. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (AutoCreateRefTerm = "InSettingPrimaryRow"))
	void SetSettingPrimaryRow(const FSettingsPrimary& InSettingPrimaryRow);

	/** Returns the main setting widget (the outer of this subwidget). */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	class USettingsWidget* GetSettingsWidget() const;
	USettingsWidget& GetSettingsWidgetChecked() const;

	/** Sets the main settings widget for this subwidget. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget")
	void SetSettingsWidget(USettingsWidget* InSettingsWidget);

protected:
	/** The Size Box widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USizeBox> SizeBoxWidget = nullptr;

	/** The widget that shows the caption text of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> CaptionWidget = nullptr;

	/** The setting primary row of this widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, DisplayName = "Setting Primary Row"))
	FSettingsPrimary SettingPrimaryRowInternal = FSettingsPrimary::EmptyPrimary;

	/** The main settings widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<class USettingsWidget> SettingsWidgetInternal = nullptr;
};

/**
 * The sub-widget of Button settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingButton : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual button widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UButton* GetButtonWidget() const { return ButtonWidget; }

	/** Returns the slate button. */
	FORCEINLINE TSharedPtr<class SButton> GetSlateButton() const { return SlateButtonInternal.Pin(); }

protected:
	/** The slate button.*/
	TWeakPtr<class SButton> SlateButtonInternal = nullptr;

	/** The actual button widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> ButtonWidget = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the Button Widget is pressed.
	 * @see USettingButton::OnSettingButtonPressed */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnButtonPressed();
};

/**
 * The sub-widget of Checkbox settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCheckbox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual checkbox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UCheckBox* GetCheckboxWidget() const { return CheckboxWidget; }

	/** Returns the slate checkbox. */
	FORCEINLINE TSharedPtr<class SCheckBox> GetSlateCheckbox() const { return SlateCheckboxInternal.Pin(); }

protected:
	/** The slate checkbox.*/
	TWeakPtr<class SCheckBox> SlateCheckboxInternal = nullptr;

	/** The actual checkbox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UCheckBox> CheckboxWidget = nullptr;

	/** Called after the underlying slate widget is constructed.
 	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the checked state has changed.
	 * @see USettingCheckbox::CheckboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnCheckStateChanged(bool bIsChecked);
};

/**
 * The sub-widget of Combobox settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCombobox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual combobox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UComboBoxString* GetComboboxWidget() const { return ComboboxWidget; }

	typedef SComboBox<TSharedPtr<FString>> SComboboxString;

	/** Returns the slate combobox. */
	FORCEINLINE TSharedPtr<SComboboxString> GetSlateCombobox() const { return SlateComboboxInternal.Pin(); }

	/** Returns true if combobox is opened. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE bool IsComboboxOpened() const { return bIsComboboxOpenedInternal; }

protected:
	/** The slate combobox.*/
	TWeakPtr<SComboboxString> SlateComboboxInternal = nullptr;

	/** The actual combobox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UComboBoxString> ComboboxWidget = nullptr;

	/** Is true if combobox is currently opened in Settings. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, DisplayName = "Is Combobox Opened"))
	bool bIsComboboxOpenedInternal = false;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is executed every tick when widget is enabled. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Called when a new item is selected in the combobox
	 * @see USettingCheckbox::ComboboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** Called when the combobox is opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnMenuOpenChanged();
};

/**
 * The sub-widget of Slider settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingSlider : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual slider widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class USlider* GetSliderWidget() const { return SliderWidget; }

	/** Returns the slate slider. */
	FORCEINLINE TSharedPtr<class SSlider> GetSlateSlider() const { return SlateSliderInternal.Pin(); }

protected:
	/** The slate slider.*/
	TWeakPtr<class SSlider> SlateSliderInternal = nullptr;

	/** The actual slider widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USlider> SliderWidget = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Invoked when the mouse is released and a capture ends. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnMouseCaptureEnd();

	/** Called when the value is changed by slider or typing.
	 * @see USettingCheckbox::SliderWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected))
	void OnValueChanged(float Value);
};

/**
 * The sub-widget of Text Line settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingTextLine : public USettingSubWidget
{
	GENERATED_BODY()

public:
};

/**
 * The sub-widget of User Input settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingUserInput : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual Editable Text Box widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UEditableTextBox* GetEditableTextBox() const { return EditableTextBox; }

	/** Returns current text set in the Editable Text Box. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	void GetEditableText(FText& OutText) const;

	/** Set new text programmatically instead of by the user. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (AutoCreateRefTerm = "InText"))
	void SetEditableText(const FText& InText);

	/** Returns the slate editable text box. */
	FORCEINLINE TSharedPtr<class SEditableTextBox> GetSlateEditableTextBox() const { return SlateEditableTextBoxInternal.Pin(); }

protected:
	/** The slate editable text box.*/
	TWeakPtr<class SEditableTextBox> SlateEditableTextBoxInternal = nullptr;

	/** The actual Editable Text Box widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UEditableTextBox> EditableTextBox = nullptr;

	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called whenever the text is changed programmatically or interactively by the user.
	 * @see USettingCheckbox::EditableTextBoxInternal */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, AutoCreateRefTerm = "Text"))
	void OnTextChanged(const FText& Text);
};

/**
 * The sub-widget of the custom widget settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCustomWidget : public USettingSubWidget
{
	GENERATED_BODY()

public:
};

/**
* The sub-widget of the Scrollbox widget settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingScrollBox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual ScrollBox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Sub-Widget")
	FORCEINLINE class UScrollBox* GetScrollBoxWidget() const { return ScrollBoxWidget; }

	/** Returns the slate ScrollBox. */
	FORCEINLINE TSharedPtr<class SScrollBox> GetSlateScrollBox() const { return SlateScrollBoxInternal.Pin(); }

protected:
	/** The slate ScrollBox.*/
	TWeakPtr<class SScrollBox> SlateScrollBoxInternal = nullptr;

	/** The actual ScrollBox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings Widget Constructor|Sub-Widget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UScrollBox> ScrollBoxWidget = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;
};
