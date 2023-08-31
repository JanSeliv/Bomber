// Copyright (c) Yevhenii Selivanov

#include "UI/Input/InputButtonWidget.h"
//---
#include "Data/SettingsDataAsset.h"
#include "DataAssets/MyInputMappingContext.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "Subsystems/SoundsSubsystem.h"
#include "UI/SettingsWidget.h"
//---
#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InputButtonWidget)

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
	const FKey& LastKey = GetCurrentKey();

	bool bMapped = false;
	if (!UPlayerInputDataAsset::Get().IsMappedKey(NewKey))
	{
		bMapped = UInputUtilsLibrary::RemapKeyInContext(InputContextInternal, MappableDataInternal, NewKey);
	}

	if (!bMapped)
	{
		// Remapping is failed, reset the key back
		checkf(InputKeySelector, TEXT("%s: 'InputKeySelector' is null"), *FString(__FUNCTION__));
		InputKeySelector->SetSelectedKey(LastKey);
		return;
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
	if (!ensureMsgf(InputKeySelector, TEXT("%s: 'InputKeySelector' is not set as BindWidget"), *FString(__FUNCTION__))
	    || !ensureMsgf(CaptionWidget, TEXT("%s: 'CaptionWidget' is not set as BindWidget"), *FString(__FUNCTION__))
	    || !ensureMsgf(SettingsWidgetInternal, TEXT("%s: 'SettingsWidgetInternal' is null"), *FString(__FUNCTION__)))
	{
		return;
	}

	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();
	const FButtonThemeData& ButtonThemeData = SettingsDataAsset.GetButtonThemeData();

	// Update the text style of the button
	// Note: const_cast is used since SetTextStyle() accepts reference inside for its slate widget, so new button style can't be copy constructed
	FTextBlockStyle& TextStyleRef = const_cast<FTextBlockStyle&>(InputKeySelector->GetTextStyle());
	TextStyleRef.SetFont(MiscThemeData.TextAndCaptionFont);
	TextStyleRef.SetColorAndOpacity(MiscThemeData.TextAndCaptionColor);
	InputKeySelector->SetTextStyle(TextStyleRef);

	// Update the widget style of the button
	// Note: const_cast is used since SetButtonStyle() accepts reference inside  for its slate widget, so new text style can't be copy constructed
	FButtonStyle& ButtonStyleRef = const_cast<FButtonStyle&>(InputKeySelector->GetButtonStyle());
	ButtonStyleRef.Normal = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Normal);
	ButtonStyleRef.Hovered = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Hovered);
	ButtonStyleRef.Pressed = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Pressed);
	ButtonStyleRef.Disabled = SettingsWidgetInternal->GetButtonBrush(ESettingsButtonState::Disabled);
	ButtonStyleRef.NormalPadding = ButtonThemeData.Padding;
	ButtonStyleRef.PressedPadding = ButtonThemeData.PressedPadding;
	InputKeySelector->SetButtonStyle(ButtonStyleRef);

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
