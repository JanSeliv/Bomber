// Copyright 2021 Yevhenii Selivanov.

#include "GameFramework/MyGameUserSettings.h"
//---
#include "Globals/SingletonLibrary.h"
//---
#include "Engine/DataTable.h"
#if WITH_EDITOR //[include]
#include "DataTableEditorUtils.h"
#endif // WITH_EDITOR

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
void USettingsDataAsset::BindOnDataTableChanged(
	const FOnDataTableChanged& EventToBind) const
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	if (!USingletonLibrary::IsEditorNotPieWorld()
	    || !SettingsDataTableInternal
	    || !EventToBind.IsBound())
	{
		return;
	}

	UDataTable::FOnDataTableChanged& OnDataTableChangedDelegate = SettingsDataTableInternal->OnDataTableChanged();
	OnDataTableChangedDelegate.AddLambda([EventToBind]()
	{
		EventToBind.Execute();
	});
#endif // WITH_EDITOR
}

//
void UMyGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

#if WITH_EDITOR // [IsEditorNotPieWorld]
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	if (USingletonLibrary::IsEditorNotPieWorld()
	    && SettingsDataAsset)
	{
		USettingsDataAsset::FOnDataTableChanged OnDataTableChanged;
		OnDataTableChanged.BindDynamic(this, &ThisClass::OnDataTableChanged);
		SettingsDataAsset->BindOnDataTableChanged(OnDataTableChanged);
	}
#endif	  // WITH_EDITOR
}

// Called whenever the data of a table has changed, this calls the OnDataTableChanged() delegate and per-row callbacks
void UMyGameUserSettings::OnDataTableChanged()
{
#if WITH_EDITOR // [IsEditorNotPieWorld]
	const USettingsDataAsset* SettingsDataAsset = USingletonLibrary::GetSettingsDataAsset();
	if (!USingletonLibrary::IsEditorNotPieWorld()
	    || !ensureMsgf(SettingsDataAsset, TEXT("ASSERT: 'SettingsDataAsset' is not valid")))
	{
		return;
	}

	UDataTable* SettingsDataTable = SettingsDataAsset->SettingsDataTableInternal;
	if (!ensureMsgf(SettingsDataTable, TEXT("ASSERT: 'SettingsDataTable' is not valid")))
	{
		return;
	}

	TMap<FName, FSettingsRow> SettingsTableRows;
	SettingsDataAsset->GenerateSettingsArray(SettingsTableRows);
	if (!ensureMsgf(SettingsTableRows.Num(), TEXT("ASSERT: 'SettingsTableRows' is empty")))
	{
		return;
	}

	for (const auto& SettingsTableRowIt : SettingsTableRows)
	{
		const FSettingsRow& RowValue = SettingsTableRowIt.Value;
		const FSettingsFunction& Setter = RowValue.Setter;
		if (!Setter.FunctionName.IsNone()
		    && Setter.FunctionClass)
		{
			UFunction* FoundSetter = Setter.FunctionClass->FindFunctionByName(Setter.FunctionName, EIncludeSuperFlag::ExcludeSuper);
			if (FoundSetter)
			{
				FOnSetter OnSetter;
				OnSetter.BindUFunction(this, Setter.FunctionName);
				OnSetter.Execute(1996);
			}
		}

		// Set row name by specified tag
		const FName RowKey = SettingsTableRowIt.Key;
		const FName RowValueTag = RowValue.Tag.GetTagName();
		static const FString EmptyRowName = FString("NewRow");
		if (!RowValueTag.IsNone()                            // Tag is not empty
		    && (RowKey != RowValueTag                        // New tag name
		        || RowKey.ToString().Contains(EmptyRowName)) // New row
		    && !SettingsTableRows.Contains(RowValueTag))     // Unique tag
		{
			FDataTableEditorUtils::RenameRow(SettingsDataTable, RowKey, RowValueTag);
		}
	}
#endif	  // WITH_EDITOR
}
