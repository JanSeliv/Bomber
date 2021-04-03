// Copyright 2021 Yevhenii Selivanov.

#include "GameFramework/MyGameUserSettings.h"
//---
#include "Globals/SingletonLibrary.h"
//---
#include "Engine/DataTable.h"

#if WITH_EDITOR //[include]
#include "DataTableEditorUtils.h"
#endif // WITH_EDITOR

// Empty settings row
const FSettingsRow FSettingsRow::EmptyRow = FSettingsRow();

// Returns the settings data asset
const USettingsDataAsset& USettingsDataAsset::Get()
{
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	checkf(SettingsDataAsset, TEXT("The Settings Data Asset is not valid"));
	return *SettingsDataAsset;
}

//
void USettingsDataAsset::GenerateSettingsArray(TMap<FName, FSettingsRow>& OutRows) const
{
	if (!ensureMsgf(SettingsDataTableInternal, TEXT("ASSERT: 'SettingsDataTableInternal' is not valid")))
	{
		return;
	}

	const TMap<FName, uint8*>& RowMap = SettingsDataTableInternal->GetRowMap();
	OutRows.Empty();
	OutRows.Reserve(RowMap.Num());
	for (const auto& RowIt : RowMap)
	{
		if (const auto FoundRowPtr = reinterpret_cast<FSettingsRow*>(RowIt.Value))
		{
			const FSettingsRow& SettingsTableRow = *FoundRowPtr;
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
	checkf(MyGameUserSettings, TEXT("The My Game User Settings is not valid"));
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

	const FSettingsRow* FoundRow = SettingsTableRowsInternal.Find(TagName);
	if (!FoundRow)
	{
		return nullptr;
	}

	const FSettingsFunction& ObjectContext = FoundRow->ObjectContext;
	if (ObjectContext.FunctionName.IsNone()
	    || !ObjectContext.FunctionClass)
	{
		return nullptr;
	}

	UObject* ClassDefaultObject = ObjectContext.FunctionClass->ClassDefaultObject;
	if (!ClassDefaultObject)
	{
		return nullptr;
	}

	FOnObjectContext OnObjectContext;
	OnObjectContext.BindUFunction(ClassDefaultObject, ObjectContext.FunctionName);
	if (OnObjectContext.IsBoundToObject(ClassDefaultObject))
	{
		UObject* OutVal = OnObjectContext.Execute();
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

	const FSettingsRow* FoundRow = SettingsTableRowsInternal.Find(TagName);
	if (!FoundRow)
	{
		return;
	}

	const FSettingsFunction& Setter = FoundRow->Setter;
	if (Setter.FunctionName.IsNone()
	    || !Setter.FunctionClass)
	{
		return;
	}

	UObject* ObjectContext = GetObjectContext(TagName);
	if (!ObjectContext)
	{
		return;
	}

	FOnSetter OnSetter;
	OnSetter.BindUFunction(ObjectContext, Setter.FunctionName);
	if (OnSetter.IsBoundToObject(ObjectContext))
	{
		OnSetter.Execute(InValue);
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

	const FSettingsRow* FoundRow = SettingsTableRowsInternal.Find(TagName);
	if (!FoundRow)
	{
		return INDEX_NONE;
	}

	const FSettingsFunction& Getter = FoundRow->Getter;
	if (Getter.FunctionName.IsNone()
	    || !Getter.FunctionClass)
	{
		return INDEX_NONE;
	}

	UObject* ObjectContext = GetObjectContext(TagName);
	if (!ObjectContext)
	{
		return INDEX_NONE;
	}

	FOnGetter OnGetter;
	OnGetter.BindUFunction(ObjectContext, Getter.FunctionName);
	if (OnGetter.IsBoundToObject(ObjectContext))
	{
		const int32 OutVal = OnGetter.Execute();
		return OutVal;
	}

	return INDEX_NONE;
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
FSettingsRow UMyGameUserSettings::FindSettingsTableRow(FName TagName) const
{
	FSettingsRow SettingsRow = FSettingsRow::EmptyRow;
	if (const FSettingsRow* SettingsRowPtr = SettingsTableRowsInternal.Find(TagName))
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
	for (const auto& SettingsTableRowIt : SettingsTableRowsInternal)
	{
		const FSettingsRow& RowValue = SettingsTableRowIt.Value;
		const FName RowKey = SettingsTableRowIt.Key;
		const FName RowValueTag = RowValue.Tag.GetTagName();
		if (!RowValueTag.IsNone()     // Tag is not empty
		    && RowKey != RowValueTag) // New tag name
		{
			FDataTableEditorUtils::RenameRow(SettingsDataTable, RowKey, RowValueTag);
		}
	}
#endif	  // WITH_EDITOR
}
