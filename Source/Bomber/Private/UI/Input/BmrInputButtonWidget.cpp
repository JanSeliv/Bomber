// Copyright (c) Yevhenii Selivanov

#include "UI/Input/BmrInputButtonWidget.h"

// Bomber
#include "Data/SettingsDataAsset.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "DataAssets/BmrPlayerInputDataAsset.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "Subsystems/BmrSoundsSubsystem.h"
#include "UI/SettingsWidget.h"

// UE
#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrInputButtonWidget)

// Sets this button to let player remap input specified in mappable data
void UBmrInputButtonWidget::InitButton(const FPlayerKeyMapping& InMappableData, const UBmrInputMappingContext* InInputMappingContext)
{
	if (!ensureMsgf(InMappableData.GetAssociatedInputAction(), TEXT("%s: 'InMappableData.Action' is not valid"), *FString(__FUNCTION__))
	    || !ensureMsgf(InInputMappingContext, TEXT("ASSERT: 'InInputMappingContext' is not valid")))
	{
		return;
	}

	MappableData = InMappableData;
	InputContext = InInputMappingContext;
}

// Sets specified key for the current input key selector
void UBmrInputButtonWidget::SetCurrentKey(const FKey& NewKey)
{
	const FKey& LastKey = GetCurrentKey();

	bool bMapped = false;
	if (!UBmrPlayerInputDataAsset::Get().IsMappedKey(this, NewKey))
	{
		bMapped = UInputUtilsLibrary::RemapKeyInContext(this, InputContext, MappableData.GetAssociatedInputAction(), MappableData.GetCurrentKey(), NewKey);
	}

	if (!bMapped)
	{
		// Remapping is failed, reset the key back
		checkf(InputKeySelector, TEXT("%s: 'InputKeySelector' is null"), *FString(__FUNCTION__));
		InputKeySelector->SetSelectedKey(LastKey);
		return;
	}

	MappableData.SetCurrentKey(NewKey);
}

// Called after the underlying slate widget is constructed
void UBmrInputButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensureMsgf(InputKeySelector, TEXT("%s: 'InputKeySelector' is not set as BindWidget"), *FString(__FUNCTION__)))
	{
		return;
	}

	InputKeySelector->SetSelectedKey(MappableData.GetCurrentKey());
	InputKeySelector->OnKeySelected.AddUniqueDynamic(this, &UBmrInputButtonWidget::OnKeySelected);
	InputKeySelector->OnIsSelectingKeyChanged.AddUniqueDynamic(this, &UBmrInputButtonWidget::OnIsSelectingKeyChanged);

	UpdateStyle();
}

// Sets the style for this button
void UBmrInputButtonWidget::UpdateStyle()
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
	CaptionWidget->SetText(MappableData.GetDisplayName());
	CaptionWidget->SetFont(MiscThemeData.TextAndCaptionFont);
	CaptionWidget->SetColorAndOpacity(MiscThemeData.TextAndCaptionColor);

	// Update padding specified in data table of this setting
	SetPadding(PrimaryDataInternal.Padding);
}

// Called whenever a new key is selected by the user
void UBmrInputButtonWidget::OnKeySelected(FInputChord SelectedKey)
{
	SetCurrentKey(SelectedKey.Key);
}

// Called whenever the key selection mode starts or stops
void UBmrInputButtonWidget::OnIsSelectingKeyChanged()
{
	UBmrSoundsSubsystem::Get().PlayUIClickSFX();
}
