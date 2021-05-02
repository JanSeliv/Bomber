// Copyright 2021 Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "GameFramework/MyGameUserSettings.h"
#include "Globals/SingletonLibrary.h"
#include "Structures/SettingsRow.h"

// Returns the found row by specified tag
FSettingsPicker USettingsWidget::FindSettingsTableRow(FName TagName) const
{
	FSettingsPicker SettingsRow = FSettingsPicker::Empty;
	if (const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TagName))
	{
		SettingsRow = *SettingsRowPtr;
	}
	return MoveTemp(SettingsRow);
}

// Returns the object of chosen option
UObject* USettingsWidget::GetObjectContext(FName TagName) const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld()) // return if not in the game
	{
		// Only during the game the ProcessEvent(...) can execute a found function
		return nullptr;
	}
#endif // WITH_EDITOR

	const FSettingsPicker& FoundRow = FindSettingsTableRow(TagName);
	if (!FoundRow.IsValid())
	{
		return nullptr;
	}

	const FSettingsFunction& StaticContext = FoundRow.PrimaryData.StaticContext;
	if (StaticContext.FunctionName.IsNone()
	    || !StaticContext.FunctionClass)
	{
		return nullptr;
	}

	UObject* ClassDefaultObject = StaticContext.FunctionClass->ClassDefaultObject;
	if (!ClassDefaultObject)
	{
		return nullptr;
	}

	UMyGameUserSettings::FOnStaticContext OnStaticContext;
	OnStaticContext.BindUFunction(ClassDefaultObject, StaticContext.FunctionName);
	if (OnStaticContext.IsBoundToObject(ClassDefaultObject))
	{
		UObject* OutVal = OnStaticContext.Execute();
		return OutVal;
	}

	return nullptr;
}

// Set value to the option
void USettingsWidget::SetOption(FName TagName, int32 InValue)
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (USingletonLibrary::IsEditorNotPieWorld()) // return if not in the game
	{
		// Only during the game the ProcessEvent(...) can execute a found function
		return;
	}
#endif // WITH_EDITOR

	const FSettingsPicker& FoundRow = FindSettingsTableRow(TagName);
	if (!FoundRow.IsValid())
	{
		return;
	}

	const FSettingsFunction& Setter = FoundRow.PrimaryData.Setter;
	if (Setter.FunctionName.IsNone()
	    || !Setter.FunctionClass)
	{
		return;
	}

	UObject* StaticContext = GetObjectContext(TagName);
	if (!StaticContext)
	{
		return;
	}

	UMyGameUserSettings::FOnSetterInt OnSetterInt;
	OnSetterInt.BindUFunction(StaticContext, Setter.FunctionName);
	if (OnSetterInt.IsBoundToObject(StaticContext))
	{
		OnSetterInt.Execute(InValue);
	}
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

	const FSettingsPicker& FoundRow = FindSettingsTableRow(TagName);
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

	UObject* StaticContext = GetObjectContext(TagName);
	if (!StaticContext)
	{
		return INDEX_NONE;
	}

	UMyGameUserSettings::FOnGetterInt OnGetterInt;
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
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsDataBase* ChosenData = RowIt.Value.GetChosenSettingsData();
		if (!ChosenData)
		{
			continue;
		}

		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;

		if (ChosenData == &RowIt.Value.Button)
		{
			if (const auto Data = static_cast<const FSettingsButton*>(ChosenData))
			{
				AddButton(PrimaryData, *Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Checkbox)
		{
			if (const auto Data = static_cast<const FSettingsCheckbox*>(ChosenData))
			{
				AddCheckbox(PrimaryData, *Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Combobox)
		{
			if (const auto Data = static_cast<const FSettingsCombobox*>(ChosenData))
			{
				AddCombobox(PrimaryData, *Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Slider)
		{
			if (const auto Data = static_cast<const FSettingsSlider*>(ChosenData))
			{
				AddSlider(PrimaryData, *Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.TextSimple)
		{
			if (const auto Data = static_cast<const FSettingsTextSimple*>(ChosenData))
			{
				AddTextSimple(PrimaryData, *Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.TextInput)
		{
			if (const auto Data = static_cast<const FSettingsTextInput*>(ChosenData))
			{
				AddTextInput(PrimaryData, *Data);
				continue;
			}
		}
	}
}

//
void USettingsWidget::AddButton_Implementation(const FSettingsPrimary& Primary, const FSettingsButton& Data)
{
	//BP implementation
}

//
void USettingsWidget::AddCheckbox_Implementation(const FSettingsPrimary& Primary, const FSettingsCheckbox& Data)
{
	//BP implementation
}

//
void USettingsWidget::AddCombobox_Implementation(const FSettingsPrimary& Primary, const FSettingsCombobox& Data)
{
	//BP implementation
}

//
void USettingsWidget::AddSlider_Implementation(const FSettingsPrimary& Primary, const FSettingsSlider& Data)
{
	//BP implementation
}

//
void USettingsWidget::AddTextSimple_Implementation(const FSettingsPrimary& Primary, const FSettingsTextSimple& Data)
{
	//BP implementation
}

//
void USettingsWidget::AddTextInput_Implementation(const FSettingsPrimary& Primary, const FSettingsTextInput& Data)
{
	//BP implementation
}
