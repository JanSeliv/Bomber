// Copyright (c) Yevhenii Selivanov

#include "UI/SettingSubWidget.h"
//---
#include "UI/SettingsWidget.h"
#include "UtilsLibrary.h"
//---
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"

// Set the new setting tag for this widget
void USettingSubWidget::SetSettingTag(const FSettingTag& NewSettingTag)
{
	SettingTagInternal = NewSettingTag;
}

// Returns the custom line height for this setting
float USettingSubWidget::GetLineHeight() const
{
	return SizeBoxWidget ? SizeBoxWidget->GetMinDesiredHeight() : 0.f;
}

// Set custom line height for this setting
void USettingSubWidget::SetLineHeight(float NewLineHeight)
{
	if (SizeBoxWidget)
	{
		SizeBoxWidget->SetMinDesiredHeight(NewLineHeight);
	}
}

// Returns the caption text that is shown on UI
void USettingSubWidget::GetCaptionText(FText& OutCaptionText) const
{
	if (CaptionWidget)
	{
		OutCaptionText = CaptionWidget->GetText();
	}
}

// Set the new caption text on UI for this widget
void USettingSubWidget::SetCaptionText(const FText& NewCaptionText)
{
	if (CaptionWidget)
	{
		CaptionWidget->SetText(NewCaptionText);
	}
}

// Set the new caption text on UI for this widget
void USettingSubWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SettingsWidgetInternal = UUtilsLibrary::GetParentWidgetOfClass<USettingsWidget>(this);
	ensureAlwaysMsgf(SettingsWidgetInternal, TEXT("ASSERT: 'SettingsWidgetInternal' is nullptr"));
}

// Called after the underlying slate widget is constructed
void USettingButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonWidget)
	{
		ButtonWidget->SetClickMethod(EButtonClickMethod::PreciseClick);
		ButtonWidget->OnClicked.AddUniqueDynamic(this, &ThisClass::USettingButton::OnButtonPressed);

		SlateButtonInternal = GetSlateWidget<SButton>(ButtonWidget);
		check(SlateButtonInternal.IsValid());
	}
}

// Called when the Button Widget is pressed
void USettingButton::OnButtonPressed()
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingButtonPressed(SettingTagInternal);
}

// Called after the underlying slate widget is constructed
void USettingCheckbox::NativeConstruct()
{
	Super::NativeConstruct();

	if (CheckboxWidget)
	{
		CheckboxWidget->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged);

		SlateCheckboxInternal = GetSlateWidget<SCheckBox>(CheckboxWidget);
		check(SlateCheckboxInternal.IsValid());
	}
}

// Called when the checked state has changed
void USettingCheckbox::OnCheckStateChanged(bool bIsChecked)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingCheckbox(SettingTagInternal, bIsChecked);
}

// Called after the underlying slate widget is constructed
void USettingCombobox::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboboxWidget)
	{
		ComboboxWidget->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnSelectionChanged);

		SlateComboboxInternal = GetSlateWidget<SComboboxString>(ComboboxWidget);
		check(SlateComboboxInternal.IsValid());
	}
}

// Is executed every tick when widget is enabled
void USettingCombobox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ComboboxWidget)
	{
		return;
	}

	const bool bIsComboboxOpenedLast = bIsComboboxOpenedInternal;
	bIsComboboxOpenedInternal = ComboboxWidget->IsOpen();

	if (bIsComboboxOpenedLast != bIsComboboxOpenedInternal)
	{
		OnMenuOpenChanged();
	}
}

// Called when the combobox is opened or closed/
void USettingCombobox::OnMenuOpenChanged()
{
	// Play the sound
	if (SettingsWidgetInternal)
	{
		SettingsWidgetInternal->PlayUIClickSFX();
	}
}

// Called when a new item is selected in the combobox
void USettingCombobox::OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!SettingsWidgetInternal
		|| !ComboboxWidget)
	{
		return;
	}

	const int32 SelectedIndex = ComboboxWidget->GetSelectedIndex();
	SettingsWidgetInternal->SetSettingComboboxIndex(SettingTagInternal, SelectedIndex);
}

// Called after the underlying slate widget is constructed
void USettingSlider::NativeConstruct()
{
	Super::NativeConstruct();

	if (SliderWidget)
	{
		SliderWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnValueChanged);
		SliderWidget->OnMouseCaptureEnd.AddUniqueDynamic(this, &ThisClass::OnMouseCaptureEnd);

		SlateSliderInternal = GetSlateWidget<SSlider>(SliderWidget);
		check(SlateSliderInternal.IsValid());
	}
}

// Invoked when the mouse is released and a capture ends
void USettingSlider::OnMouseCaptureEnd()
{
	// Play the sound
	if (SettingsWidgetInternal)
	{
		SettingsWidgetInternal->PlayUIClickSFX();
	}
}

// Called when the value is changed by slider or typing
void USettingSlider::OnValueChanged(float Value)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingSlider(SettingTagInternal, Value);
}

// Returns current text set in the Editable Text Box
void USettingUserInput::GetEditableText(FText& OutText) const
{
	if (EditableTextBox)
	{
		OutText = EditableTextBox->GetText();
	}
}

// Set new text programmatically instead of by the user
void USettingUserInput::SetEditableText(const FText& InText)
{
	if (!EditableTextBox
		|| InText.EqualTo(EditableTextBox->GetText()))
	{
		return;
	}

	EditableTextBox->SetText(InText);
}

// Called after the underlying slate widget is constructed
void USettingUserInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox)
	{
		EditableTextBox->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnTextChanged);

		SlateEditableTextBoxInternal = GetSlateWidget<SEditableTextBox>(EditableTextBox);
		check(SlateEditableTextBoxInternal.IsValid());
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
	SettingsWidgetInternal->SetSettingUserInput(SettingTagInternal, MewValue);
}

// Called after the underlying slate widget is constructed
void USettingScrollBox::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ScrollBoxWidget)
	{
		SlateScrollBoxInternal = GetSlateWidget<SScrollBox>(ScrollBoxWidget);
		check(SlateScrollBoxInternal.IsValid());
	}
}
