// Copyright (c) Yevhenii Selivanov

#include "UI/Input/InputControlsWidget.h"
//---
#include "DataAssets/MyInputMappingContext.h"
#include "DataAssets/PlayerInputDataAsset.h"
#include "UI/SettingsWidget.h"
#include "UI/Input/InputCategoryWidget.h"
//---
#include "Components/ScrollBox.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(InputControlsWidget)

// Called after the underlying slate widget is constructed
void UInputControlsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateAllInputCategories();

	SetPadding(SettingPrimaryRowInternal.Padding);
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
		FInputCategoryData::GetCategoriesDataFromMappings(*InputContextIt, /*Out*/InputCategoriesData);

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
