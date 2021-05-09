// Copyright 2021 Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Globals/SingletonLibrary.h"
#include "UI/MyHUD.h"

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

// Returns the found row by specified tag
FSettingsPicker USettingsWidget::FindSettingRow(FName TagName) const
{
	FSettingsPicker FoundRow = FSettingsPicker::Empty;
	if (const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TagName))
	{
		FoundRow = *SettingsRowPtr;
	}
	return FoundRow;
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

// Returns the name of found tag by specified function
FName USettingsWidget::GetTagNameByFunction(const FSettingsFunction& Function) const
{
	TArray<FSettingsPicker> Rows;
	SettingsTableRowsInternal.GenerateValueArray(Rows);
	const FSettingsPicker* FoundRow = Rows.FindByPredicate([&Function](const FSettingsPicker& Row) { return Row.PrimaryData.Getter == Function || Row.PrimaryData.Setter == Function; });
	return FoundRow ? FoundRow->PrimaryData.Tag.GetTagName() : FGameplayTag::EmptyTag.GetTagName();
}

// Set value to the option by tag
void USettingsWidget::SetSettingValue(FName TagName, const FString& Value)
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (!FoundRow.IsValid())
	{
		return;
	}

	const FSettingsDataBase* ChosenData = FoundRow.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	if (ChosenData == &FoundRow.Button)
	{
		SetButtonPressed(TagName);
	}
	else if (ChosenData == &FoundRow.Checkbox)
	{
		const bool NewValue = Value.ToBool();
		SetCheckbox(TagName, NewValue);
	}
	else if (ChosenData == &FoundRow.Combobox)
	{
		if (Value.IsNumeric())
		{
			const int32 NewValue = FCString::Atoi(*Value);
			SetComboboxIndex(TagName, NewValue);
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
			SetComboboxMembers(TagName, NewMembers);
		}
	}
	else if (ChosenData == &FoundRow.Slider)
	{
		const float NewValue = FCString::Atof(*Value);
		SetSlider(TagName, NewValue);
	}
	else if (ChosenData == &FoundRow.TextSimple)
	{
		const FText NewValue = FText::FromString(Value);
		SetTextSimple(TagName, NewValue);
	}
	else if (ChosenData == &FoundRow.TextInput)
	{
		const FText NewValue = FText::FromString(Value);
		SetTextInput(TagName, NewValue);
	}
}

// Press button
void USettingsWidget::SetButtonPressed_Implementation(FName TagName)
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.Button.OnButtonPressed.ExecuteIfBound();
	}

	// BP implementation
}

// Toggle checkbox
void USettingsWidget::SetCheckbox_Implementation(FName TagName, bool InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.Checkbox.bIsSet = InValue;
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.Checkbox.OnSetterBool.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Set chosen member index for a combobox
void USettingsWidget::SetComboboxIndex_Implementation(FName TagName, int32 InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.Combobox.ChosenMemberIndex = InValue;
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.Combobox.OnSetterInt.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Set new members for a combobox
void USettingsWidget::SetComboboxMembers_Implementation(FName TagName, const TArray<FText>& InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.Combobox.Members = InValue;
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.Combobox.OnSetMembers.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Set current value for a slider
void USettingsWidget::SetSlider_Implementation(FName TagName, float InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		static constexpr float MinValue = 0.f;
		static constexpr float MaxValue = 1.f;
		FoundRow.Slider.ChosenValue = FMath::Clamp(InValue, MinValue, MaxValue);
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.Slider.OnSetterFloat.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Set new text
void USettingsWidget::SetTextSimple_Implementation(FName TagName, const FText& InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.PrimaryData.Caption = InValue;
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.TextSimple.OnSetterText.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Set new text for an input box
void USettingsWidget::SetTextInput_Implementation(FName TagName, const FText& InValue)
{
	FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (FoundRow.IsValid())
	{
		FoundRow.PrimaryData.Caption = InValue;
		SettingsTableRowsInternal.Emplace(TagName, MoveTemp(FoundRow));

		FoundRow.TextInput.OnSetterText.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Returns is a checkbox toggled
bool USettingsWidget::GetCheckboxValue(FName TagName) const
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
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
int32 USettingsWidget::GetComboboxIndex(FName TagName) const
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
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
TArray<FText> USettingsWidget::GetComboboxMembers(FName TagName) const
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
	TArray<FText> Value;
	if (FoundRow.IsValid())
	{
		const FSettingsCombobox& Data = FoundRow.Combobox;
		Value = Data.Members;

		const USettingTemplate::FOnGetMembers& Getter = Data.OnGetMembers;
		if (Getter.IsBound())
		{
			Value = Getter.Execute();
		}
	}
	return Value;
}

// Get current value of a slider [0...1]
float USettingsWidget::GetSliderValue(FName TagName) const
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
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
FText USettingsWidget::GetTextValue(FName TagName) const
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);
	FText Value = TEXT_NONE;
	if (FoundRow.IsValid())
	{
		Value = FoundRow.PrimaryData.Caption;

		const auto Data = static_cast<const FSettingsTextSimple*>(FoundRow.GetChosenSettingsData());
		if (Data)
		{
			const USettingTemplate::FOnGetterText& Getter = Data->OnGetterText;
			if (Getter.IsBound())
			{
				Value = Getter.Execute();
			}
		}
	}
	return Value;
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

	OnVisibilityChanged.AddUniqueDynamic(this, &ThisClass::OnVisibilityChange);

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen escape input to go back to the main menu
	if (AMyHUD* MyHUD = USingletonLibrary::GetMyHUD())
	{
		MyHUD->OnGoUIBack.AddUniqueDynamic(this, &ThisClass::CloseSettings);
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

// Called when the visibility has changed
void USettingsWidget::OnVisibilityChange_Implementation(ESlateVisibility InVisibility)
{
	// BP Implementation
	//...
}

// Bind and set static object delegate
void USettingsWidget::TryBindStaticContext(FSettingsPrimary& Primary)
{
	const FSettingsFunction& StaticContext = Primary.StaticContext;
	if (StaticContext.FunctionName.IsNone()
	    || !StaticContext.FunctionClass)
	{
		return;
	}

	UObject* ClassDefaultObject = StaticContext.FunctionClass->ClassDefaultObject;
	if (!ClassDefaultObject)
	{
		return;
	}

	USettingTemplate::FOnStaticContext OnStaticContext;
	OnStaticContext.BindUFunction(ClassDefaultObject, StaticContext.FunctionName);
	if (!OnStaticContext.IsBound())
	{
		return;
	}

	if (UObject* ContextObject = OnStaticContext.Execute())
	{
		Primary.StaticContextObject = ContextObject;
		if (UClass* ContextClass = ContextObject->GetClass())
		{
			for (TFieldIterator<UFunction> It(ContextClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
			{
				if (const UFunction* FunctionIt = *It)
				{
					FName FunctionNameIt = FunctionIt->GetFName();
					Primary.StaticContextFunctionList.Emplace(MoveTemp(FunctionNameIt));
				}
			}
		}
	}
}

// Bind on text getter and setter
void USettingsWidget::TryBindTextFunctions(FSettingsPrimary& Primary, FSettingsTextSimple& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterText.BindUFunction(StaticContextObject, GetterFunctionName);
			Primary.Caption = GetTextValue(Primary.Tag.GetTagName());
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterText.BindUFunction(StaticContextObject, SetterFunctionName);
			Data.OnSetterText.ExecuteIfBound(Primary.Caption);
		}
	}
}

// Save and close the settings widget
void USettingsWidget::CloseSettings()
{
	if (!IsVisible())
	{
		// Widget is already closed
		return;
	}

	SaveSettings();

	SetVisibility(ESlateVisibility::Collapsed);

	if (UMyGameUserSettings* MyGameUserSettings = USingletonLibrary::GetMyGameUserSettings())
	{
		MyGameUserSettings->ApplySettings(true);
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
		AddButton(PrimaryData, Setting.Button);
	}
	else if (ChosenData == &Setting.Checkbox)
	{
		AddCheckbox(PrimaryData, Setting.Checkbox);
	}
	else if (ChosenData == &Setting.Combobox)
	{
		AddCombobox(PrimaryData, Setting.Combobox);
	}
	else if (ChosenData == &Setting.Slider)
	{
		AddSlider(PrimaryData, Setting.Slider);
	}
	else if (ChosenData == &Setting.TextSimple)
	{
		AddTextSimple(PrimaryData, Setting.TextSimple);
	}
	else if (ChosenData == &Setting.TextInput)
	{
		AddTextInput(PrimaryData, Setting.TextInput);
	}
}

// Add button on UI
void USettingsWidget::AddButton(FSettingsPrimary& Primary, FSettingsButton& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnButtonPressed.BindUFunction(StaticContextObject, SetterFunctionName);
		}
	}

	AddButtonBP(Primary, Data);

	// BP implementation
	// ...
}

// Add checkbox on UI
void USettingsWidget::AddCheckbox(FSettingsPrimary& Primary, FSettingsCheckbox& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterBool.BindUFunction(StaticContextObject, GetterFunctionName);
			Data.bIsSet = GetCheckboxValue(Primary.Tag.GetTagName());
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterBool.BindUFunction(StaticContextObject, SetterFunctionName);
			Data.OnSetterBool.ExecuteIfBound(Data.bIsSet);
		}
	}

	AddCheckboxBP(Primary, Data);

	// BP implementation
	// ...
}

// Add combobox on UI
void USettingsWidget::AddCombobox(FSettingsPrimary& Primary, FSettingsCombobox& Data)
{
	const FName TagName = Primary.Tag.GetTagName();
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetMembersFunctionName = Data.GetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetMembersFunctionName))
		{
			Data.OnGetMembers.BindUFunction(StaticContextObject, GetMembersFunctionName);
			Data.Members = GetComboboxMembers(TagName);
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
			Data.ChosenMemberIndex = GetComboboxIndex(TagName);
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterInt.BindUFunction(StaticContextObject, SetterFunctionName);
			Data.OnSetterInt.ExecuteIfBound(Data.ChosenMemberIndex);
		}
	}

	AddComboboxBP(Primary, Data);

	// BP implementation
	// ...
}

// Add slider on UI
void USettingsWidget::AddSlider(FSettingsPrimary& Primary, FSettingsSlider& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnGetterFloat.BindUFunction(StaticContextObject, GetterFunctionName);
			Data.ChosenValue = GetSliderValue(Primary.Tag.GetTagName());
		}

		const FName SetterFunctionName = Primary.Setter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetterFunctionName))
		{
			Data.OnSetterFloat.BindUFunction(StaticContextObject, SetterFunctionName);
			Data.OnSetterFloat.ExecuteIfBound(Data.ChosenValue);
		}
	}

	AddSliderBP(Primary, Data);

	// BP implementation
	// ...
}

// Add simple text on UI
void USettingsWidget::AddTextSimple(FSettingsPrimary& Primary, FSettingsTextSimple& Data)
{
	TryBindTextFunctions(Primary, Data);

	AddTextSimpleBP(Primary, Data);

	// BP implementation
	// ...
}

// Add text input on UI
void USettingsWidget::AddTextInput(FSettingsPrimary& Primary, FSettingsTextInput& Data)
{
	TryBindTextFunctions(Primary, Data);

	AddTextInputBP(Primary, Data);

	// BP implementation
	// ...
}

// Starts adding settings on the next column
void USettingsWidget::StartNextColumn_Implementation()
{
	// BP implementation
	// ...
}
