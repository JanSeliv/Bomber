// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "MyDataTable/MyDataTable.h"
//---
#include "Structures/SettingsRow.h"
//---
#include "SettingsDataTable.generated.h"

/**
 * Settings data table with FSettingsRow members.
 * Provides additional in-editor functionality like automatic set the key name by specified setting tag.
 */
UCLASS(BlueprintType)
class SETTINGSWIDGETCONSTRUCTOR_API USettingsDataTable : public UMyDataTable
{
	GENERATED_BODY()

public:
	/** Default constructor to set members as FSettingsRow. */
	USettingsDataTable();

	/** Returns the table rows.
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GetSettingRows(TMap<FName, FSettingsRow>& OutRows) const { GetRows(OutRows); }

protected:
#if WITH_EDITOR
	/** Called on every change in this data table to automatic set the key name by specified setting tag. */
	virtual void OnThisDataTableChanged(FName RowKey, const uint8& RowData) override;
#endif // WITH_EDITOR
};
