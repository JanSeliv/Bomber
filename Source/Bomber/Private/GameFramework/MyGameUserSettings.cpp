// Copyright 2021 Yevhenii Selivanov.

#include "GameFramework/MyGameUserSettings.h"
//---
#include "Globals/SingletonLibrary.h"
//---
#include "Engine/DataTable.h"

#if WITH_EDITOR //[include]
#include "DataTableEditorUtils.h"
#endif // WITH_EDITOR

// Returns the settings data asset
const USettingsDataAsset& USettingsDataAsset::Get()
{
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	checkf(SettingsDataAsset, TEXT("The Settings Data Asset is not valid"));
	return *SettingsDataAsset;
}

//
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

// Returns the game user settings
UMyGameUserSettings& UMyGameUserSettings::Get()
{
	UMyGameUserSettings* MyGameUserSettings = USingletonLibrary::GetMyGameUserSettings();
	checkf(MyGameUserSettings, TEXT("My Game User Settings is not valid"));
	return *MyGameUserSettings;
}

UObject* UMyGameUserSettings::GetObjectContext(FName TagName) const
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

	FOnStaticContext OnStaticContext;
	OnStaticContext.BindUFunction(ClassDefaultObject, StaticContext.FunctionName);
	if (OnStaticContext.IsBoundToObject(ClassDefaultObject))
	{
		UObject* OutVal = OnStaticContext.Execute();
		return OutVal;
	}

	return nullptr;
}

//
void UMyGameUserSettings::SetOption(FName TagName, int32 InValue)
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

	FOnSetterInt OnSetterInt;
	OnSetterInt.BindUFunction(StaticContext, Setter.FunctionName);
	if (OnSetterInt.IsBoundToObject(StaticContext))
	{
		OnSetterInt.Execute(InValue);
	}
}

int32 UMyGameUserSettings::GetOption(FName TagName) const
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

	FOnGetterInt OnGetterInt;
	OnGetterInt.BindUFunction(StaticContext, Getter.FunctionName);
	if (OnGetterInt.IsBoundToObject(StaticContext))
	{
		const int32 OutVal = OnGetterInt.Execute();
		return OutVal;
	}

	return INDEX_NONE;
}

//
void UMyGameUserSettings::InitSettings_Implementation()
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsDataBase* ChosenData = RowIt.Value.GetChosenSettingsData();
		if (!ChosenData)
		{
			continue;
		}

		if (ChosenData == &RowIt.Value.Button)
		{
			if (const auto Data = static_cast<const FSettingsButton*>(ChosenData))
			{
				AddSettingsButton(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.ButtonsRow)
		{
			if (const auto Data = static_cast<const FSettingsButtonsRow*>(ChosenData))
			{
				AddSettingsButtonsRow(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Checkbox)
		{
			if (const auto Data = static_cast<const FSettingsCheckbox*>(ChosenData))
			{
				AddSettingsCheckbox(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Combobox)
		{
			if (const auto Data = static_cast<const FSettingsCombobox*>(ChosenData))
			{
				AddSettingsCombobox(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.Slider)
		{
			if (const auto Data = static_cast<const FSettingsSlider*>(ChosenData))
			{
				AddSettingsSlider(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.TextSimple)
		{
			if (const auto Data = static_cast<const FSettingsTextSimple*>(ChosenData))
			{
				AddSettingsTextSimple(*Data);
				continue;
			}
		}

		if (ChosenData == &RowIt.Value.TextInput)
		{
			if (const auto Data = static_cast<const FSettingsTextInput*>(ChosenData))
			{
				AddSettingsTextInput(*Data);
				continue;
			}
		}
	}
}

//
void UMyGameUserSettings::AddSettingsButton_Implementation(const FSettingsButton& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsButtonsRow_Implementation(const FSettingsButtonsRow& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsCheckbox_Implementation(const FSettingsCheckbox& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsCombobox_Implementation(const FSettingsCombobox& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsSlider_Implementation(const FSettingsSlider& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsTextSimple_Implementation(const FSettingsTextSimple& Data)
{
	//BP implementation
}

//
void UMyGameUserSettings::AddSettingsTextInput_Implementation(const FSettingsTextInput& Data)
{
	//BP implementation
}


//
void UMyGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	USettingsDataAsset::Get().GenerateSettingsArray(SettingsTableRowsInternal);

#if WITH_EDITOR // [IsEditorNotPieWorld]
	// Notify settings for any change in the settings data table
	if (USingletonLibrary::IsEditorNotPieWorld())
	{
		// Bind only once
		static USettingsDataAsset::FOnDataTableChanged OnDataTableChanged;
		if (!OnDataTableChanged.IsBound())
		{
			OnDataTableChanged.BindDynamic(this, &ThisClass::OnDataTableChanged);
			USettingsDataAsset::Get().BindOnDataTableChanged(OnDataTableChanged);
		}
	}
#endif // WITH_EDITOR
}

//
FSettingsPicker UMyGameUserSettings::FindSettingsTableRow(FName TagName) const
{
	FSettingsPicker SettingsRow = FSettingsPicker::Empty;
	if (const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TagName))
	{
		SettingsRow = *SettingsRowPtr;
	}
	return MoveTemp(SettingsRow);
}

// Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks
void UMyGameUserSettings::OnDataTableChanged()
{
#if WITH_EDITOR  // [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld())
	{
		return;
	}

	UDataTable* SettingsDataTable = USettingsDataAsset::Get().GetSettingsDataTable();
	if (!ensureMsgf(SettingsDataTable, TEXT("ASSERT: 'SettingsDataTable' is not valid")))
	{
		return;
	}

	// Set row name by specified tag
	USettingsDataAsset::Get().GenerateSettingsArray(SettingsTableRowsInternal);
	for (const TTuple<FName, FSettingsPicker>& SettingsTableRowIt : SettingsTableRowsInternal)
	{
		const FSettingsPicker& SettingsRow = SettingsTableRowIt.Value;

		const FName RowKey = SettingsTableRowIt.Key;
		const FName RowValueTag = SettingsTableRowIt.Value.PrimaryData.Tag.GetTagName();
		if (!RowValueTag.IsNone()     // Tag is not empty
		    && RowKey != RowValueTag) // New tag name
		{
			FDataTableEditorUtils::RenameRow(SettingsDataTable, RowKey, RowValueTag);
		}
	}
#endif	  // WITH_EDITOR
}
