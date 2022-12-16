// Copyright (c) Yevhenii Selivanov

#include "UI/InputControlsWidget.h"
//---
#include "WidgetUtilsLibrary.h"
#include "Globals/MyInputMappingContext.h"
#include "Globals/PlayerInputDataAsset.h"
#include "Data/SettingsDataAsset.h"
#include "UI/SettingsWidget.h"
#include "SoundsManager.h"
//---
#include "Components/VerticalBox.h"
#include "Components/InputKeySelector.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "UtilityLibraries/SingletonLibrary.h"
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

// Sets the style for the Input Key Selector
void UInputButtonWidget::UpdateStyle()
{
	SInputKeySelector* SlateInputKeySelector = UWidgetUtilsLibrary::GetSlateWidget<SInputKeySelector>(InputKeySelector).Get();
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
}

// Called whenever a new key is selected by the user
void UInputButtonWidget::OnKeySelected(FInputChord SelectedKey)
{
	SetCurrentKey(SelectedKey.Key);
}

// Called whenever the key selection mode starts or stops
void UInputButtonWidget::OnIsSelectingKeyChanged()
{
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}
}

// Returns all categories from the specified input mapping context
void FInputCategoryData::GetCategoriesDataFromMappings(const UMyInputMappingContext* InInputMappingContext, TArray<FInputCategoryData>& OutInputCategoriesData)
{
	checkf(InInputMappingContext, TEXT("%s: 'InInputMappingContext' is null"), *FString(__FUNCTION__));
	TArray<FEnhancedActionKeyMapping> AllMappings;
	InInputMappingContext->GetAllMappings(/*Out*/AllMappings);

	// Find all categories in every mapping
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		const FText& DisplayCategory = MappingIt.PlayerMappableOptions.DisplayCategory;
		FInputCategoryData* CategoryData = OutInputCategoriesData.FindByPredicate([&DisplayCategory](const FInputCategoryData& CategoryDataIt)
		{
			return CategoryDataIt.CategoryName.EqualTo(DisplayCategory);
		});

		if (CategoryData)
		{
			// Add mapping to existing category
			CategoryData->Mappings.AddUnique(MappingIt);
		}
		else
		{
			// Is not created yet, add new category
			FInputCategoryData NewCategoryData;
			NewCategoryData.Mappings.AddUnique(MappingIt);
			NewCategoryData.CategoryName = DisplayCategory;
			NewCategoryData.InputMappingContext = InInputMappingContext;
			OutInputCategoriesData.Emplace(MoveTemp(NewCategoryData));
		}
	}
}

// Sets the input context to be represented by this widget
void UInputCategoryWidget::CreateInputButtons(const FInputCategoryData& InInputCategoryData)
{
	if (!ensureMsgf(InputButtonClassInternal, TEXT("%s: 'Input Button Class' is not set, can not create input buttons"), *FString(__FUNCTION__)))
	{
		return;
	}

	InputCategoryDataInternal = InInputCategoryData;

	for (const FEnhancedActionKeyMapping& MappableDataIt : InInputCategoryData.Mappings)
	{
		FSettingsPrimary NewPrimaryRow = SettingPrimaryRowInternal;
		UInputButtonWidget* InputButtonWidget = GetSettingsWidgetChecked().CreateSettingSubWidget<UInputButtonWidget>(NewPrimaryRow, InputButtonClassInternal);

		InputButtonsInternal.Emplace(InputButtonWidget);
		InputButtonWidget->InitButton(MappableDataIt, InInputCategoryData.InputMappingContext);
	}
}

// Called after the underlying slate widget is constructed
void UInputCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AttachInputButtons();
}

// Adds all input buttons to the root of this widget
void UInputCategoryWidget::AttachInputButtons()
{
	if (!ensureMsgf(VerticalBoxInputButtons, TEXT("%s: 'VerticalBoxInputButtons' is not set as BindWidget"), *FString(__FUNCTION__)))
	{
		return;
	}

	for (UInputButtonWidget* InputButtonIt : InputButtonsInternal)
	{
		VerticalBoxInputButtons->AddChild(InputButtonIt);
	}
}

// Called after the underlying slate widget is constructed
void UInputControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateAllInputCategories();
}

// Adds input categories for each mapping context
void UInputControlsWidget::CreateAllInputCategories()
{
	if (!InputCategoriesInternal.IsEmpty()
	    || !ensureMsgf(InputCategoryClassInternal, TEXT("ASSERT: 'Input Category Class' is null")))
	{
		return;
	}

	TArray<const UMyInputMappingContext*> OutGameplayInputContexts;
	UPlayerInputDataAsset::Get().GetAllGameplayInputContexts(OutGameplayInputContexts);

	for (const UMyInputMappingContext* InputContextIt : OutGameplayInputContexts)
	{
		// Inside each input context, there could be different input categories
		TArray<FInputCategoryData> InputCategoriesData;
		FInputCategoryData::GetCategoriesDataFromMappings(InputContextIt, /*Out*/InputCategoriesData);

		for (const FInputCategoryData& InputCategoryDataIt : InputCategoriesData)
		{
			FSettingsPrimary NewPrimaryRow = SettingPrimaryRowInternal;
			NewPrimaryRow.Caption = InputCategoryDataIt.CategoryName;
			UInputCategoryWidget* InputCategoryWidget = GetSettingsWidgetChecked().CreateSettingSubWidget<UInputCategoryWidget>(NewPrimaryRow, InputCategoryClassInternal);

			InputCategoriesInternal.Emplace(InputCategoryWidget);
			InputCategoryWidget->CreateInputButtons(InputCategoryDataIt);

			checkf(ScrollBoxInputCategories, TEXT("%s: 'ScrollBoxInputCategories' is not set as BindWidget"), *FString(__FUNCTION__));
			ScrollBoxInputCategories->AddChild(InputCategoryWidget);
		}
	}
}
