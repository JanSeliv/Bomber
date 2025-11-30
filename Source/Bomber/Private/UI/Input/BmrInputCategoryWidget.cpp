// Copyright (c) Yevhenii Selivanov

#include "UI/Input/BmrInputCategoryWidget.h"

// Bomber
#include "Data/SettingsDataAsset.h"
#include "DataAssets/BmrInputMappingContext.h"
#include "MyUtilsLibraries/InputUtilsLibrary.h"
#include "UI/Input/BmrInputButtonWidget.h"
#include "UI/SettingsWidget.h"

// UE
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrInputCategoryWidget)

// Returns all categories from the specified input mapping context
void FInputCategoryData::GetCategoriesDataFromMappings(const UObject& WorldContext, const UBmrInputMappingContext& InInputMappingContext, TArray<FInputCategoryData>& OutInputCategoriesData)
{
	TArray<FPlayerKeyMapping> AllMappings;
	UInputUtilsLibrary::GetAllMappingsInContext(&WorldContext, &InInputMappingContext, /*out*/ AllMappings);

	// Find all categories in every mapping
	for (const FPlayerKeyMapping& MappingIt : AllMappings)
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
void UBmrInputCategoryWidget::CreateInputButtons(const FInputCategoryData& InInputCategoryData)
{
	if (!ensureMsgf(InputButtonClass, TEXT("%s: 'Input Button Class' is not set, can not create input buttons"), *FString(__FUNCTION__)))
	{
		return;
	}

	InputCategoryData = InInputCategoryData;

	for (const FPlayerKeyMapping& MappableDataIt : InInputCategoryData.Mappings)
	{
		FSettingsPrimary NewPrimaryRow = PrimaryDataInternal;
		UBmrInputButtonWidget* InputButtonWidget = GetSettingsWidgetChecked().CreateSettingSubWidget<UBmrInputButtonWidget>(NewPrimaryRow, InputButtonClass);

		InputButtons.Emplace(InputButtonWidget);
		InputButtonWidget->InitButton(MappableDataIt, InInputCategoryData.InputMappingContext);
	}
}

// Called after the underlying slate widget is constructed
void UBmrInputCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AttachInputButtons();

	UpdateStyle();
}

// Sets the style for this input category
void UBmrInputCategoryWidget::UpdateStyle()
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
void UBmrInputCategoryWidget::AttachInputButtons()
{
	if (!ensureMsgf(VerticalBoxInputButtons, TEXT("%s: 'VerticalBoxInputButtons' is not set as BindWidget"), *FString(__FUNCTION__)))
	{
		return;
	}

	for (UBmrInputButtonWidget* InputButtonIt : InputButtons)
	{
		VerticalBoxInputButtons->AddChild(InputButtonIt);
	}
}
