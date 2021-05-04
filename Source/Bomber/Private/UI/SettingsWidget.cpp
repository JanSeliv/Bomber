// Copyright 2021 Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Globals/SingletonLibrary.h"

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

// Set value to the option by tag
void USettingsWidget::SetSettingValue(FName TagName, const FString& Value)
{
	const FSettingsPicker FoundRow = FindSettingRow(TagName);;
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

		FoundRow.TextSimple.OnSetterText.ExecuteIfBound(InValue);
	}

	// BP implementation
}

// Return the value of the option
int32 USettingsWidget::GetOption(FName TagName) const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld()) // return if not in the game
	{
		// Only during the game the ProcessEvent(...) can execute a found function
		return INDEX_NONE;
	}
#endif // WITH_EDITOR

	const FSettingsPicker FoundRow = FindSettingRow(TagName);
	if (!FoundRow.IsValid())
	{
		return INDEX_NONE;
	}

	const FSettingsFunction& Getter = FoundRow.PrimaryData.Getter;
	if (Getter.FunctionName.IsNone()
	    || !Getter.FunctionClass)
	{
		return INDEX_NONE;
	}

	UObject* StaticContext = FoundRow.PrimaryData.StaticContextObject.Get();
	if (!StaticContext)
	{
		return INDEX_NONE;
	}

	USettingTemplate::FOnGetterInt OnGetterInt;
	OnGetterInt.BindUFunction(StaticContext, Getter.FunctionName);
	if (OnGetterInt.IsBoundToObject(StaticContext))
	{
		const int32 OutVal = OnGetterInt.Execute();
		return OutVal;
	}

	return INDEX_NONE;
}

// Called after the underlying slate widget is constructed
void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide that widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	ConstructSettings();
}

//
void USettingsWidget::ConstructSettings()
{
	USettingsDataAsset::Get().GenerateSettingsArray(SettingsTableRowsInternal);
	for (TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		AddSetting(RowIt.Value);
	}
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
		if (const UClass* ContextClass = ContextObject->GetClass())
		{
			ContextClass->GenerateFunctionList(Primary.StaticContextFunctionList);
		}
	}
}

// Bind on text getter and setter
void USettingsWidget::TryBindTextFunctions(const FSettingsPrimary& Primary, FSettingsTextSimple& Data)
{
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
}

// Add setting on UI.
void USettingsWidget::AddSetting(FSettingsPicker& Setting)
{
	FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	FSettingsPrimary& PrimaryData = Setting.PrimaryData;
	TryBindStaticContext(PrimaryData);

	if (ChosenData == &Setting.Button)
	{
		if (FSettingsButton* Data = static_cast<FSettingsButton*>(ChosenData))
		{
			AddButton(PrimaryData, *Data);
		}
	}
	else if (ChosenData == &Setting.Checkbox)
	{
		if (FSettingsCheckbox* Data = static_cast<FSettingsCheckbox*>(ChosenData))
		{
			AddCheckbox(PrimaryData, *Data);
		}
	}
	else if (ChosenData == &Setting.Combobox)
	{
		if (FSettingsCombobox* Data = static_cast<FSettingsCombobox*>(ChosenData))
		{
			AddCombobox(PrimaryData, *Data);
		}
	}
	else if (ChosenData == &Setting.Slider)
	{
		if (FSettingsSlider* Data = static_cast<FSettingsSlider*>(ChosenData))
		{
			AddSlider(PrimaryData, *Data);
		}
	}
	else if (ChosenData == &Setting.TextSimple)
	{
		if (FSettingsTextSimple* Data = static_cast<FSettingsTextSimple*>(ChosenData))
		{
			AddTextSimple(PrimaryData, *Data);
		}
	}
	else if (ChosenData == &Setting.TextInput)
	{
		if (FSettingsTextInput* Data = static_cast<FSettingsTextInput*>(ChosenData))
		{
			AddTextInput(PrimaryData, *Data);
		}
	}
}

// Add button on UI
void USettingsWidget::AddButton_Implementation(FSettingsPrimary& Primary, FSettingsButton& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
		const FName GetterFunctionName = Primary.Getter.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetterFunctionName))
		{
			Data.OnButtonPressed.BindUFunction(StaticContextObject, GetterFunctionName);
		}
	}

	//BP implementation
}

// Add checkbox on UI
void USettingsWidget::AddCheckbox_Implementation(FSettingsPrimary& Primary, FSettingsCheckbox& Data)
{
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

	//BP implementation
}

// Add combobox on UI
void USettingsWidget::AddCombobox_Implementation(FSettingsPrimary& Primary, FSettingsCombobox& Data)
{
	if (UObject* StaticContextObject = Primary.StaticContextObject.Get())
	{
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

		const FName GetMembersFunctionName = Data.GetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(GetMembersFunctionName))
		{
			Data.OnGetMembers.BindUFunction(StaticContextObject, GetMembersFunctionName);
		}

		const FName SetMembersFunctionName = Data.SetMembers.FunctionName;
		if (Primary.StaticContextFunctionList.Contains(SetMembersFunctionName))
		{
			Data.OnSetMembers.BindUFunction(StaticContextObject, SetMembersFunctionName);
		}
	}

	//BP implementation
}

// Add slider on UI
void USettingsWidget::AddSlider_Implementation(FSettingsPrimary& Primary, FSettingsSlider& Data)
{
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

	//BP implementation
}

// Add simple text on UI
void USettingsWidget::AddTextSimple_Implementation(FSettingsPrimary& Primary, FSettingsTextSimple& Data)
{
	TryBindTextFunctions(Primary, Data);

	//BP implementation
}

// Add text input on UI
void USettingsWidget::AddTextInput_Implementation(FSettingsPrimary& Primary, FSettingsTextInput& Data)
{
	TryBindTextFunctions(Primary, Data);

	//BP implementation
}
