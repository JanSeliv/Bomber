// Copyright (c) Yevhenii Selivanov

#include "UI/Input/InputButtonWidget.h"
//---
#include "Subsystems/SoundsSubsystem.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "Data/SettingsDataAsset.h"
#include "DataAssets/MyInputMappingContext.h"
#include "UI/SettingsWidget.h"
//---
#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "Widgets/Input/SInputKeySelector.h"

// Sets this button to let player remap input specified in mappable data
void UInputButtonWidget::InitButton(const FEnhancedActionKeyMapping& InMappableData, const UMyInputMappingContext* InInputMappingContext)
{
	if (!ensureMsgf(InMappableData.Action, TEXT("%s: 'InMappableData.Action' is not valid"), *FString(__FUNCTION__))
	    || !ensureMsgf(InInputMappingContext, TEXT("ASSERT: 'InInputMappingContext' is not valid")))
	{
		return;
	}

	MappableDataInternal = InMappableData;
	InputContextInternal = InInputMappingContext;
}

// Sets specified key for the current input key selector
void UInputButtonWidget::SetCurrentKey(const FKey& NewKey)
{
	const FKey LastKey = GetCurrentKey();

	const bool bMapped = UMyInputMappingContext::RemapKey(InputContextInternal, MappableDataInternal, NewKey);

	if (!bMapped)
	{
		// Remapping is failed, reset the key back
		checkf(InputKeySelector, TEXT("%s: 'InputKeySelector' is null"), *FString(__FUNCTION__));
		InputKeySelector->SetSelectedKey(LastKey);
	}

	MappableDataInternal.Key = NewKey;
}

// Called after the underlying slate widget is constructed
void UInputButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensureMsgf(InputKeySelector, TEXT("%s: 'InputKeySelector' is not set as BindWidget"), *FString(__FUNCTION__)))
	{
		return;
	}

	InputKeySelector->SetSelectedKey(MappableDataInternal.Key);
	InputKeySelector->OnKeySelected.AddUniqueDynamic(this, &UInputButtonWidget::OnKeySelected);
	InputKeySelector->OnIsSelectingKeyChanged.AddUniqueDynamic(this, &UInputButtonWidget::OnIsSelectingKeyChanged);

	UpdateStyle();
}

// Sets the style for this button
void UInputButtonWidget::UpdateStyle()
{
	SInputKeySelector* SlateInputKeySelector = FWidgetUtilsLibrary::GetSlateWidget<SInputKeySelector>(InputKeySelector).Get();
	if (!ensureMsgf(SlateInputKeySelector, TEXT("%s: 'SlateInputKeySelector' is not valid"), *FString(__FUNCTION__))
	    || !ensureMsgf(InputKeySelector, TEXT("%s: 'InputKeySelector' is not set as BindWidget"), *FString(__FUNCTION__))
	    || !ensureMsgf(CaptionWidget, TEXT("%s: 'CaptionWidget' is not set as BindWidget"), *FString(__FUNCTION__))
	    || !ensureMsgf(SettingsWidgetInternal, TEXT("%s: 'SettingsWidgetInternal' is null"), *FString(__FUNCTION__)))
	{
		return;
	}

	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();
	const FButtonThemeData& ButtonThemeData = SettingsDataAsset.GetButtonThemeData();

	// Update the text style of the button
	FTextBlockStyle& TextStyleRef = InputKeySelector->TextStyle;
	TextStyleRef.SetFont(MiscThemeData.TextAndCaptionFont);
	TextStyleRef.SetColorAndOpacity(MiscThemeData.TextAndCaptionColor);
	SlateInputKeySelector->SetTextStyle(&TextStyleRef);

	// Update the widget style of the button
	FButtonStyle& WidgetStyleRef = InputKeySelector->WidgetStyle;
	WidgetStyleRef.Normal = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Normal);
	WidgetStyleRef.Hovered = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Hovered);
	WidgetStyleRef.Pressed = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Pressed);
	WidgetStyleRef.Disabled = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Disabled);
	WidgetStyleRef.NormalPadding = ButtonThemeData.Padding;
	WidgetStyleRef.PressedPadding = ButtonThemeData.PressedPadding;
	SlateInputKeySelector->SetButtonStyle(&WidgetStyleRef);

	// Update text
	CaptionWidget->SetText(MappableDataInternal.PlayerMappableOptions.DisplayName);
	CaptionWidget->SetFont(MiscThemeData.TextAndCaptionFont);
	CaptionWidget->SetColorAndOpacity(MiscThemeData.TextAndCaptionColor);

	// Update padding specified in data table of this setting
	SetPadding(SettingPrimaryRowInternal.Padding);
}

// Called whenever a new key is selected by the user
void UInputButtonWidget::OnKeySelected(FInputChord SelectedKey)
{
	SetCurrentKey(SelectedKey.Key);
}

// Called whenever the key selection mode starts or stops
void UInputButtonWidget::OnIsSelectingKeyChanged()
{
	USoundsSubsystem::Get().PlayUIClickSFX();
}
