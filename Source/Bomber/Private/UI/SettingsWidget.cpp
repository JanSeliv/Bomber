// Copyright 2021 Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "SoundsManager.h"
#include "GameFramework/MyGameUserSettings.h"
#include "Globals/SingletonLibrary.h"
#include "UI/MyHUD.h"
#include "UI/SettingSubWidget.h"

// Returns the settings data asset
const USettingsDataAsset& USettingsDataAsset::Get()
{
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	checkf(SettingsDataAsset, TEXT("The Settings Data Asset is not valid"));
	return *SettingsDataAsset;
}

// Returns the table rows.
void USettingsDataAsset::GenerateSettingsArray(TMap<FName, FSettingsPicker>& OutRows) const
{
	if (!ensureMsgf(SettingsDataTableInternal, TEXT("ASSERT: 'SettingsDataTableInternal' is not valid")))
	{
		return;
	}

	const TMap<FName, uint8*>& RowMap = SettingsDataTableInternal->GetRowMap();
	OutRows.Empty();
	OutRows.Reserve(RowMap.Num());
	for (const TTuple<FName, uint8*>& RowIt : RowMap)
	{
		if (const auto FoundRowPtr = reinterpret_cast<FSettingsRow*>(RowIt.Value))
		{
			const FSettingsPicker& SettingsTableRow = FoundRowPtr->SettingsPicker;
			const FName RowName = RowIt.Key;
			OutRows.Emplace(RowName, SettingsTableRow);
		}
	}
}

// Get a multicast delegate that is called any time the data table changes
void USettingsDataAsset::BindOnDataTableChanged(const FOnDataTableChanged& EventToBind) const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld()
	    || !SettingsDataTableInternal
	    || !EventToBind.IsBound())
	{
		return;
	}

	UDataTable::FOnDataTableChanged& OnDataTableChangedDelegate = SettingsDataTableInternal->OnDataTableChanged();
	OnDataTableChangedDelegate.AddLambda([EventToBind]() { EventToBind.ExecuteIfBound(); });
#endif // WITH_EDITOR
}

// Try to find the setting row
const FSettingsPicker& USettingsWidget::FindSettingRow(FName PotentialTagName) const
{
	if (PotentialTagName.IsNone())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = &FSettingsPicker::Empty;

	// Find row by specified substring
	const FString TagSubString(PotentialTagName.ToString());
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FString TagStringIt(RowIt.Key.ToString());
		if (TagStringIt.Contains(TagSubString))
		{
			FoundRow = &RowIt.Value;
			break;
		}
	}

	return *FoundRow;
}

// Returns the found row by specified tag
const FSettingsPicker& USettingsWidget::GetSettingRow(const FGameplayTag& SettingTag) const
{
	if (!SettingTag.IsValid())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = SettingsTableRowsInternal.Find(SettingTag.GetTagName());
	return FoundRow ? *FoundRow : FSettingsPicker::Empty;
}

// Save all settings into their configs
void USettingsWidget::SaveSettings()
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		if (UObject* ContextObject = RowIt.Value.PrimaryData.StaticContextObject.Get())
		{
			ContextObject->SaveConfig();
		}
	}
}

// Update settings on UI
void USettingsWidget::UpdateSettings(const FGameplayTagContainer& SettingsToUpdate)
{
	if (SettingsToUpdate.IsEmpty())
	{
		return;
	}

	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPicker& Setting = RowIt.Value;
		const FGameplayTag& SettingTag = Setting.PrimaryData.Tag;
		if (!SettingTag.MatchesAny(SettingsToUpdate))
		{
			continue;
		}

		const FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
		if (!ChosenData)
		{
			continue;
		}

		if (ChosenData == &Setting.Checkbox)
		{
			const bool NewValue = GetCheckboxValue(SettingTag);
			SetSettingCheckbox(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.Combobox)
		{
			const int32 NewValue = GetComboboxIndex(SettingTag);
			SetSettingComboboxIndex(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.Slider)
		{
			const float NewValue = GetSliderValue(SettingTag);
			SetSettingSlider(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.TextLine)
		{
			FText NewValue = TEXT_NONE;
			GetTextLineValue(SettingTag, /*Out*/NewValue);
			SetSettingTextLine(SettingTag, NewValue);
		}
		else if (ChosenData == &Setting.UserInput)
		{
			const FName NewValue = GetUserInputValue(SettingTag);
			SetSettingUserInput(SettingTag, NewValue);
		}
	}
}

// Returns the name of found tag by specified function
const FGameplayTag& USettingsWidget::GetTagByFunctionPicker(const FFunctionPicker& FunctionPicker) const
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;
		if (PrimaryData.Getter == FunctionPicker
		    || PrimaryData.Setter == FunctionPicker)
		{
			return PrimaryData.Tag;
		}
	}

	return FGameplayTag::EmptyTag;
}

// Set value to the option by tag
void USettingsWidget::SetSettingValue(FName TagName, const FString& Value)
{
	const FSettingsPicker& FoundRow = FindSettingRow(TagName);
	if (!FoundRow.IsValid())
	{
		return;
	}

	const FSettingsDataBase* ChosenData = FoundRow.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	const FGameplayTag& Tag = FoundRow.PrimaryData.Tag;
	if (!Tag.IsValid())
	{
		return;
	}

	if (ChosenData == &FoundRow.Button)
	{
		SetSettingButtonPressed(Tag);
	}
	else if (ChosenData == &FoundRow.Checkbox)
	{
		const bool NewValue = Value.ToBool();
		SetSettingCheckbox(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.Combobox)
	{
		if (Value.IsNumeric())
		{
			const int32 NewValue = FCString::Atoi(*Value);
			SetSettingComboboxIndex(Tag, NewValue);
		}
		else
		{
			static const FString Delimiter = TEXT(",");
			TArray<FString> SeparatedStrings;
			Value.ParseIntoArray(SeparatedStrings, *Delimiter);

			TArray<FText> NewMembers;
			NewMembers.Reserve(SeparatedStrings.Num());
			for (FString& StringIt : SeparatedStrings)
			{
				NewMembers.Emplace(FText::FromString(MoveTemp(StringIt)));
			}
			SetSettingComboboxMembers(Tag, NewMembers);
		}
	}
	else if (ChosenData == &FoundRow.Slider)
	{
		const float NewValue = FCString::Atof(*Value);
		SetSettingSlider(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.TextLine)
	{
		const FText NewValue = FText::FromString(Value);
		SetSettingTextLine(Tag, NewValue);
	}
	else if (ChosenData == &FoundRow.UserInput)
	{
		const FName NewValue = *Value;
		SetSettingUserInput(Tag, NewValue);
	}
}

// Press button
void USettingsWidget::SetSettingButtonPressed(const FGameplayTag& ButtonTag)
{
	if (!ButtonTag.IsValid())
	{
		return;
	}

	const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ButtonTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Button.OnButtonPressed.ExecuteIfBound();

	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);
}

// Toggle checkbox
void USettingsWidget::SetSettingCheckbox(const FGameplayTag& CheckboxTag, bool InValue)
{
	if (!CheckboxTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(CheckboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	bool& bIsSetRef = SettingsRowPtr->Checkbox.bIsSet;
	if (bIsSetRef == InValue)
	{
		return;
	}

	bIsSetRef = InValue;
	SettingsRowPtr->Checkbox.OnSetterBool.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetCheckbox(CheckboxTag, InValue);
}

// Set chosen member index for a combobox
void USettingsWidget::SetSettingComboboxIndex(const FGameplayTag& ComboboxTag, int32 InValue)
{
	if (!ComboboxTag.IsValid())
	{
		return;
	}

	if (InValue == INDEX_NONE)
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ComboboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	int32& ChosenMemberIndexRef = SettingsRowPtr->Combobox.ChosenMemberIndex;
	if (ChosenMemberIndexRef == InValue)
	{
		return;
	}

	ChosenMemberIndexRef = InValue;
	SettingsRowPtr->Combobox.OnSetterInt.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetComboboxIndex(ComboboxTag, InValue);
}

// Set new members for a combobox
void USettingsWidget::SetSettingComboboxMembers(const FGameplayTag& ComboboxTag, const TArray<FText>& InValue)
{
	if (!ComboboxTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ComboboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Combobox.Members = InValue;
	SettingsRowPtr->Combobox.OnSetMembers.ExecuteIfBound(InValue);

	// BP implementation
	SetComboboxMembers(ComboboxTag, InValue);
}

// Set current value for a slider
void USettingsWidget::SetSettingSlider(const FGameplayTag& SliderTag, float InValue)
{
	if (!SliderTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(SliderTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	static constexpr float MinValue = 0.f;
	static constexpr float MaxValue = 1.f;
	const float NewValue = FMath::Clamp(InValue, MinValue, MaxValue);
	float& ChosenValueRef = SettingsRowPtr->Slider.ChosenValue;
	if (ChosenValueRef == NewValue)
	{
		return;
	}

	ChosenValueRef = NewValue;
	SettingsRowPtr->Slider.OnSetterFloat.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetSlider(SliderTag, InValue);
}

// Set new text
void USettingsWidget::SetSettingTextLine(const FGameplayTag& TextLineTag, const FText& InValue)
{
	if (!TextLineTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TextLineTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsPrimary& PrimaryRef = SettingsRowPtr->PrimaryData;
	FText& CaptionRef = PrimaryRef.Caption;
	if (CaptionRef.EqualTo(InValue))
	{
		return;
	}

	CaptionRef = InValue;
	SettingsRowPtr->TextLine.OnSetterText.ExecuteIfBound(InValue);
	UpdateSettings(PrimaryRef.SettingsToUpdate);

	if (USettingTextLine* SettingTextLine = Cast<USettingTextLine>(PrimaryRef.SettingSubWidget))
	{
		SettingTextLine->SetCaptionText(InValue);
	}
}

// Set new text for an input box
void USettingsWidget::SetSettingUserInput(const FGameplayTag& UserInputTag, FName InValue)
{
	if (!UserInputTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(UserInputTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsUserInput& UserInputRef = SettingsRowPtr->UserInput;
	if (UserInputRef.UserInput.IsEqual(InValue)
	    || InValue.IsNone())
	{
		// Is not needed to update
		return;
	}

	if (UserInputRef.MaxCharactersNumber > 0)
	{
		// Limit the length of the string
		const FString NewValueStr = InValue.ToString().Left(UserInputRef.MaxCharactersNumber);
		InValue = *NewValueStr;

		if (USettingUserInput* SettingUserInput = Cast<USettingUserInput>(SettingsRowPtr->PrimaryData.SettingSubWidget.Get()))
		{
			SettingUserInput->SetEditableText(FText::FromString(NewValueStr));
		}
	}

	UserInputRef.UserInput = InValue;
	UserInputRef.OnSetterName.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetUserInput(UserInputTag, InValue);
}

// Set new custom widget for setting by specified tag
void USettingsWidget::SetSettingCustomWidget(const FGameplayTag& CustomWidgetTag, USettingCustomWidget* InCustomWidget)
{
	if (!CustomWidgetTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(CustomWidgetTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	TWeakObjectPtr<USettingSubWidget>& CustomWidgetRef = SettingsRowPtr->PrimaryData.SettingSubWidget;
	if (CustomWidgetRef == InCustomWidget)
	{
		return;
	}

	CustomWidgetRef.Reset();
	CustomWidgetRef = InCustomWidget;
	SettingsRowPtr->CustomWidget.OnSetterWidget.ExecuteIfBound(InCustomWidget);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);
}

// Returns is a checkbox toggled
bool USettingsWidget::GetCheckboxValue(const FGameplayTag& CheckboxTag) const
{
	if (!CheckboxTag.IsValid())
	{
		return false;
	}

	const FSettingsPicker& FoundRow = GetSettingRow(CheckboxTag);
	bool Value = false;
	if (FoundRow.IsValid())
	{
		const FSettingsCheckbox& Data = FoundRow.Checkbox;
		Value = Data.bIsSet;

		const USettingTemplate::FOnGetterBool& Getter = Data.OnGetterBool;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Returns chosen member index of a combobox
int32 USettingsWidget::GetComboboxIndex(const FGameplayTag& ComboboxTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(ComboboxTag);
	int32 Value = false;
	if (FoundRow.IsValid())
	{
		const FSettingsCombobox& Data = FoundRow.Combobox;
		Value = Data.ChosenMemberIndex;

		const USettingTemplate::FOnGetterInt& Getter = Data.OnGetterInt;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get all members of a combobox
void USettingsWidget::GetComboboxMembers(const FGameplayTag& ComboboxTag, TArray<FText>& OutMembers) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(ComboboxTag);
	if (FoundRow.IsValid())
	{
		const FSettingsCombobox& Data = FoundRow.Combobox;
		OutMembers = Data.Members;

		const USettingTemplate::FOnGetMembers& Getter = Data.OnGetMembers;
		if (Getter.IsBound())
		{
			Getter.Execute(OutMembers);
		}
	}
}

// Get current value of a slider [0...1]
float USettingsWidget::GetSliderValue(const FGameplayTag& SliderTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(SliderTag);
	float Value = 0.f;
	if (FoundRow.IsValid())
	{
		const FSettingsSlider& Data = FoundRow.Slider;
		Value = Data.ChosenValue;

		const USettingTemplate::FOnGetterFloat& Getter = Data.OnGetterFloat;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get current text of a simple text widget
void USettingsWidget::GetTextLineValue(const FGameplayTag& TextLineTag, FText& OutText) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(TextLineTag);
	if (FoundRow.IsValid())
	{
		OutText = FoundRow.PrimaryData.Caption;

		const USettingTemplate::FOnGetterText& Getter = FoundRow.TextLine.OnGetterText;
		if (Getter.IsBound())
		{
			Getter.Execute(OutText);
		}
	}
}

// Get current input name of the text input
FName USettingsWidget::GetUserInputValue(const FGameplayTag& UserInputTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(UserInputTag);
	FName Value = NAME_None;
	if (FoundRow.IsValid())
	{
		const FSettingsUserInput& Data = FoundRow.UserInput;
		Value = Data.UserInput;

		const USettingTemplate::FOnGetterName& Getter = Data.OnGetterName;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get custom widget of the setting by specified tag
USettingCustomWidget* USettingsWidget::GetCustomWidget(const FGameplayTag& CustomWidgetTag) const
{
	const FSettingsPicker& FoundRow = GetSettingRow(CustomWidgetTag);
	USettingCustomWidget* CustomWidget = nullptr;
	if (FoundRow.IsValid())
	{
		CustomWidget = Cast<USettingCustomWidget>(FoundRow.PrimaryData.SettingSubWidget.Get());

		const USettingTemplate::FOnGetterWidget& Getter = FoundRow.CustomWidget.OnGetterWidget;
		if (Getter.IsBound())
		{
			CustomWidget = Getter.Execute();
		}
	}
	return CustomWidget;
}

// Get setting widget object by specified tag
USettingSubWidget* USettingsWidget::GetSettingSubWidget(const FGameplayTag& SettingTag) const
{
	const FSettingsPrimary& PrimaryData = GetSettingRow(SettingTag).PrimaryData;
	return PrimaryData.IsValid() ? PrimaryData.SettingSubWidget.Get() : nullptr;
}

// Called after the underlying slate widget is constructed
void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	USettingsDataAsset::Get().GenerateSettingsArray(SettingsTableRowsInternal);

	// Set overall columns num by amount of rows that are marked to be started on next column
	TArray<FSettingsPicker> Rows;
	SettingsTableRowsInternal.GenerateValueArray(Rows);
	OverallColumnsNumInternal += Rows.FilterByPredicate([](const FSettingsPicker& Row) { return Row.PrimaryData.bStartOnNextColumn; }).Num();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	OnVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnVisibilityChange);

	// Listen until all other widgets will be initialized
	if (AMyHUD* MyHUD = USingletonLibrary::GetMyHUD())
	{
		if (MyHUD->AreWidgetInitialized())
		{
			OnWidgetsInitialized();
		}
		else
		{
			MyHUD->OnWidgetsInitialized.AddDynamic(this, &ThisClass::OnWidgetsInitialized);
		}
	}
}

// Construct all settings from the settings data table
void USettingsWidget::ConstructSettings_Implementation()
{
	// BP implementation to create subsetting widgets
	//...

	for (TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		AddSetting(RowIt.Value);
	}
}

// Is called when all game widgets are initialized to construct settings
void USettingsWidget::OnWidgetsInitialized()
{
	AMyHUD* HUD = USingletonLibrary::GetMyHUD();
	if (HUD
	    && HUD->OnWidgetsInitialized.IsAlreadyBound(this, &ThisClass::OnWidgetsInitialized))
	{
		HUD->OnWidgetsInitialized.RemoveDynamic(this, &ThisClass::OnWidgetsInitialized);
	}

	ConstructSettings();
}

// Is called when visibility is changed for this widget
void USettingsWidget::OnVisibilityChange(ESlateVisibility InVisibility)
{
	AMyHUD* MyHUD = USingletonLibrary::GetMyHUD();
	if (!MyHUD)
	{
		return;
	}

	if (InVisibility == ESlateVisibility::Visible)
	{
		MyHUD->OnClose.AddUniqueDynamic(this, &ThisClass::CloseSettings);
	}
	else if (MyHUD->OnClose.IsAlreadyBound(this, &ThisClass::CloseSettings))
	{
		MyHUD->OnClose.RemoveDynamic(this, &ThisClass::CloseSettings);
	}
}

// Bind and set static object delegate
void USettingsWidget::TryBindStaticContext(FSettingsPrimary& Primary)
{
	UObject* FoundContextObj = nullptr;
	if (UFunction* FunctionPtr = Primary.StaticContext.GetFunction())
	{
		FunctionPtr->ProcessEvent(FunctionPtr, /*Out*/&FoundContextObj);
	}

	if (!FoundContextObj)
	{
		return;
	}

	Primary.StaticContextObject = FoundContextObj;

	const UClass* ContextClass = FoundContextObj->GetClass();
	if (!ensureMsgf(ContextClass, TEXT("ASSERT: 'ContextClass' is not valid")))
	{
		return;
	}

	// Cache all functions that are contained in returned object
	for (TFieldIterator<UFunction> It(ContextClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (!FunctionIt)
		{
			continue;
		}

		const FName FunctionNameIt = FunctionIt->GetFName();
		if (!FunctionNameIt.IsNone())
		{
			Primary.StaticContextFunctionList.Emplace(FunctionNameIt);
		}
	}
}

// Creates new widget based on specified setting class and sets it to specified primary data
void USettingsWidget::CreateSettingSubWidget(FSettingsPrimary& Primary, const TSubclassOf<USettingSubWidget>& SettingSubWidgetClass)
{
	if (!SettingSubWidgetClass)
	{
		return;
	}

	USettingSubWidget* SettingSubWidget = CreateWidget<USettingSubWidget>(this, SettingSubWidgetClass);
	Primary.SettingSubWidget = SettingSubWidget;
	SettingSubWidget->SetSettingTag(Primary.Tag);
	SettingSubWidget->SetLineHeight(Primary.LineHeight);
	SettingSubWidget->SetCaptionText(Primary.Caption);
}

// Starts adding settings on the next column
void USettingsWidget::StartNextColumn_Implementation()
{
	// BP implementation
	// ...
}

// Display settings on UI
void USettingsWidget::OpenSettings()
{
	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	SetVisibility(ESlateVisibility::Visible);
}

// Save and close the settings widget
void USettingsWidget::CloseSettings()
{
	if (!IsVisible()
	    || !IsHovered())
	{
		// Widget is already closed
		return;
	}

	// Play the sound
	if (USoundsManager* SoundsManager = USingletonLibrary::GetSoundsManager())
	{
		SoundsManager->PlayUIClickSFX();
	}

	SetVisibility(ESlateVisibility::Collapsed);

	if (UMyGameUserSettings* MyGameUserSettings = USingletonLibrary::GetMyGameUserSettings())
	{
		// Will apply and save settings
		MyGameUserSettings->ApplySettings(false);
	}

	SaveSettings();
}

// Flip-flop opens and closes the Settings menu
void USettingsWidget::ToggleSettings()
{
	if (IsVisible())
	{
		CloseSettings();
	}
	else
	{
		OpenSettings();
	}
}

// Add setting on UI.
void USettingsWidget::AddSetting(FSettingsPicker& Setting)
{
	const FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	FSettingsPrimary& PrimaryData = Setting.PrimaryData;
	TryBindStaticContext(PrimaryData);

	if (Setting.PrimaryData.bStartOnNextColumn)
	{
		StartNextColumn();
	}

	if (ChosenData == &Setting.Button)
	{
		AddSettingButton(PrimaryData, Setting.Button);
	}
	else if (ChosenData == &Setting.Checkbox)
	{
		AddSettingCheckbox(PrimaryData, Setting.Checkbox);
	}
	else if (ChosenData == &Setting.Combobox)
	{
		AddSettingCombobox(PrimaryData, Setting.Combobox);
	}
	else if (ChosenData == &Setting.Slider)
	{
		AddSettingSlider(PrimaryData, Setting.Slider);
	}
	else if (ChosenData == &Setting.TextLine)
	{
		AddSettingTextLine(PrimaryData, Setting.TextLine);
	}
	else if (ChosenData == &Setting.UserInput)
	{
		AddSettingUserInput(PrimaryData, Setting.UserInput);
	}
	else if (ChosenData == &Setting.CustomWidget)
	{
		AddSettingCustomWidget(PrimaryData, Setting.CustomWidget);
	}

	UpdateSettings(FGameplayTagContainer(PrimaryData.Tag));
}

// Add button on UI
void USettingsWidget::AddSettingButton(FSettingsPrimary& Primary, FSettingsButton& Data)
{
	const TSubclassOf<USettingButton>& ButtonClass = USettingsDataAsset::Get().GetButtonClass();
	CreateSettingSubWidget(Primary, ButtonClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnButtonPressed.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddButton(Primary, Data);
}

// Add checkbox on UI
void USettingsWidget::AddSettingCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data)
{
	const TSubclassOf<USettingCheckbox>& CheckboxClass = USettingsDataAsset::Get().GetCheckboxClass();
	CreateSettingSubWidget(Primary, CheckboxClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterBool.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterBool.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCheckbox(Primary, Data);
}

// Add combobox on UI
void USettingsWidget::AddSettingCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data)
{
	const TSubclassOf<USettingCombobox>& ComboboxClass = USettingsDataAsset::Get().GetComboboxClass();
	CreateSettingSubWidget(Primary, ComboboxClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetMembersFunctionName = Data.GetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetMembersFunctionName))
		{
			Data.OnGetMembers.BindUFunction(StaticContextObject, GetMembersFunctionName);
			Data.OnGetMembers.ExecuteIfBound(Data.Members);
		}

		const FName SetMembersFunctionName = Data.SetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetMembersFunctionName))
		{
			Data.OnSetMembers.BindUFunction(StaticContextObject, SetMembersFunctionName);
			Data.OnSetMembers.ExecuteIfBound(Data.Members);
		}

		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterInt.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterInt.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCombobox(Primary, Data);
}

// Add slider on UI
void USettingsWidget::AddSettingSlider(FSettingsPrimary& Primary, FSettingsSlider& Data)
{
	const TSubclassOf<USettingSlider>& SliderClass = USettingsDataAsset::Get().GetSliderClass();
	CreateSettingSubWidget(Primary, SliderClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterFloat.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterFloat.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddSlider(Primary, Data);
}

// Add simple text on UI
void USettingsWidget::AddSettingTextLine(FSettingsPrimary& Primary, FSettingsTextLine& Data)
{
	const TSubclassOf<USettingTextLine>& TextLineClass = USettingsDataAsset::Get().GetTextLineClass();
	CreateSettingSubWidget(Primary, TextLineClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterText.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterText.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddTextLine(Primary, Data);
}

// Add text input on UI
void USettingsWidget::AddSettingUserInput(FSettingsPrimary& Primary, FSettingsUserInput& Data)
{
	const TSubclassOf<USettingUserInput>& UserInputClass = USettingsDataAsset::Get().GetUserInputClass();
	CreateSettingSubWidget(Primary, UserInputClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterName.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterName.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddUserInput(Primary, Data);
}

// Add custom widget on UI
void USettingsWidget::AddSettingCustomWidget(FSettingsPrimary& Primary, FSettingsCustomWidget& Data)
{
	CreateSettingSubWidget(Primary, Data.CustomWidgetClass);

	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterWidget.BindUFunction(StaticContextObject, GetterFunctionName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterWidget.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddCustomWidget(Primary, Data);
}
