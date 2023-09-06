// Copyright (c) Yevhenii Selivanov

#include "UI/Input/InputCategoryWidget.h"
//---
#include "Data/SettingsDataAsset.h"
#include "DataAssets/MyInputMappingContext.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "UI/SettingsWidget.h"
#include "UI/Input/InputButtonWidget.h"
//---
#include "EnhancedActionKeyMapping.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InputCategoryWidget)

// Returns all categories from the specified input mapping context
void FInputCategoryData::GetCategoriesDataFromMappings(const UMyInputMappingContext& InInputMappingContext, TArray<FInputCategoryData>& OutInputCategoriesData)
{
	TArray<FEnhancedActionKeyMapping> AllMappings;
	UInputUtilsLibrary::GetAllMappingsInContext(&InInputMappingContext, /*out*/AllMappings);

	// Find all categories in every mapping
	for (const FEnhancedActionKeyMapping& MappingIt : AllMappings)
	{
		const FText& DisplayCategory = MappingIt.GetDisplayCategory();
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
			NewCategoryData.InputMappingContext = &InInputMappingContext;
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

	UpdateStyle();
}

// Sets the style for this input category
void UInputCategoryWidget::UpdateStyle()
{
	if (!ensureMsgf(CaptionWidget, TEXT("%s: 'CaptionWidget' is not set as BindWidget"), *FString(__FUNCTION__)))
	{
		return;
	}

	// Update category text style
	if (CaptionWidget->GetText().IsEmpty())
	{
		CaptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		const FMiscThemeData& MiscThemeData = USettingsDataAsset::Get().GetMiscThemeData();
		CaptionWidget->SetFont(MiscThemeData.TextAndCaptionFont);
		CaptionWidget->SetColorAndOpacity(MiscThemeData.ThemeColorHover);
	}
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
