// Copyright (c) Yevhenii Selivanov

#include "UI/Input/BmrInputControlsWidget.h"

// Bomber
#include "DataAssets/BmrInputMappingContext.h"
#include "DataAssets/BmrPlayerInputDataAsset.h"
#include "UI/Input/BmrInputCategoryWidget.h"
#include "UI/SettingsWidget.h"

// UE
#include "Components/ScrollBox.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrInputControlsWidget)

// Called after the underlying slate widget is constructed
void UBmrInputControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateAllInputCategories();

	SetPadding(PrimaryDataInternal.Padding);
}

// Adds input categories for each mapping context
void UBmrInputControlsWidget::CreateAllInputCategories()
{
	if (!InputCategories.IsEmpty()
	    || !ensureMsgf(InputCategoryClass, TEXT("ASSERT: 'Input Category Class' is null")))
	{
		return;
	}

	TArray<const UBmrInputMappingContext*> OutGameplayInputContexts;
	UBmrPlayerInputDataAsset::Get().GetAllGameplayInputContexts(OutGameplayInputContexts);

	for (const UBmrInputMappingContext* InputContextIt : OutGameplayInputContexts)
	{
		// Inside each input context, there could be different input categories
		TArray<FInputCategoryData> InputCategoriesData;
		FInputCategoryData::GetCategoriesDataFromMappings(*this, *InputContextIt, /*Out*/ InputCategoriesData);

		for (const FInputCategoryData& InputCategoryDataIt : InputCategoriesData)
		{
			FSettingsPrimary NewPrimaryRow = PrimaryDataInternal;
			NewPrimaryRow.Caption = InputCategoryDataIt.CategoryName;
			UBmrInputCategoryWidget* InputCategoryWidget = GetSettingsWidgetChecked().CreateSettingSubWidget<UBmrInputCategoryWidget>(NewPrimaryRow, InputCategoryClass);

			InputCategories.Emplace(InputCategoryWidget);
			InputCategoryWidget->CreateInputButtons(InputCategoryDataIt);

			checkf(ScrollBoxInputCategories, TEXT("%s: 'ScrollBoxInputCategories' is not set as BindWidget"), *FString(__FUNCTION__));
			ScrollBoxInputCategories->AddChild(InputCategoryWidget);
		}
	}
}
