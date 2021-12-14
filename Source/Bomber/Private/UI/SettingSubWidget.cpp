// Copyright 2021 Yevhenii Selivanov

#include "UI/SettingSubWidget.h"
//---
#include "Globals/SingletonLibrary.h"
#include "UI/SettingsWidget.h"
//---
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

// Set the new setting tag for this widget
void USettingSubWidget::SetSettingTag(const FGameplayTag& NewSettingTag)
{
	SettingTagInternal = NewSettingTag;
}

// Returns the custom line height for this setting
float USettingSubWidget::GetLineHeight() const
{
	return SizeBoxWidgetInternal ? SizeBoxWidgetInternal->MinDesiredHeight : 0.f;
}

// Set custom line height for this setting
void USettingSubWidget::SetLineHeight(float NewLineHeight)
{
	if (SizeBoxWidgetInternal)
	{
		SizeBoxWidgetInternal->SetMinDesiredHeight(NewLineHeight);
	}
}

// Returns the caption text that is shown on UI
void USettingSubWidget::GetCaptionText(FText& OutCaptionText) const
{
	if (CaptionWidgetInternal)
	{
		OutCaptionText = CaptionWidgetInternal->GetText();
	}
}

// Set the new caption text on UI for this widget
void USettingSubWidget::SetCaptionText(const FText& NewCaptionText)
{
	if (CaptionWidgetInternal)
	{
		CaptionWidgetInternal->SetText(NewCaptionText);
	}
}

// Set the new caption text on UI for this widget
void USettingSubWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SettingsWidgetInternal = USingletonLibrary::GetSettingsWidget();
}

// Called after the underlying slate widget is constructed
void USettingButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonWidgetInternal)
	{
		ButtonWidgetInternal->OnPressed.AddUniqueDynamic(this, &ThisClass::USettingButton::OnButtonPressed);
	}
}

// Called when the Button Widget is pressed
void USettingButton::OnButtonPressed()
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetButtonPressed(SettingTagInternal);
}

// Called after the underlying slate widget is constructed
void USettingCheckbox::NativeConstruct()
{
	Super::NativeConstruct();

	if (CheckboxWidgetInternal)
	{
		CheckboxWidgetInternal->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged);
	}
}

// Called when the checked state has changed
void USettingCheckbox::OnCheckStateChanged(bool bIsChecked)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetCheckbox(SettingTagInternal, bIsChecked);
}

// Called after the underlying slate widget is constructed
void USettingCombobox::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboboxWidgetInternal)
	{
		ComboboxWidgetInternal->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnSelectionChanged);
	}
}

// Called when a new item is selected in the combobox
void USettingCombobox::OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!SettingsWidgetInternal
	    || !ComboboxWidgetInternal)
	{
		return;
	}

	const int32 SelectedIndex = ComboboxWidgetInternal->GetSelectedIndex();
	SettingsWidgetInternal->SetComboboxIndex(SettingTagInternal, SelectedIndex);
}

// Called after the underlying slate widget is constructed
void USettingSlider::NativeConstruct()
{
	Super::NativeConstruct();

	if (SliderWidgetInternal)
	{
		SliderWidgetInternal->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnValueChanged);
	}
}

// Called when the value is changed by slider or typing
void USettingSlider::OnValueChanged(float Value)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSlider(SettingTagInternal, Value);
}

// Returns current text set in the Editable Text Box
void USettingUserInput::GetEditableText(FText& OutText) const
{
	if (EditableTextBoxInternal)
	{
		OutText = EditableTextBoxInternal->GetText();
	}
}

// Set new text programmatically instead of by the user
void USettingUserInput::SetEditableText(const FText& InText)
{
	if (!EditableTextBoxInternal
	    || InText.EqualTo(EditableTextBoxInternal->GetText()))
	{
		return;
	}

	EditableTextBoxInternal->SetText(InText);
}

// Called after the underlying slate widget is constructed
void USettingUserInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBoxInternal)
	{
		EditableTextBoxInternal->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnTextChanged);
	}
}

// Called whenever the text is changed programmatically or interactively by the user
void USettingUserInput::OnTextChanged(const FText& Text)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	const FName MewValue(Text.ToString());
	SettingsWidgetInternal->SetUserInput(SettingTagInternal, MewValue);
}
