// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Engine/DataTable.h"
//---
#include "Structures/SettingsRow.h"
//---
#include "SettingsDataTable.generated.h"

/**
 * Settings data table with FSettingsRow members.
 * Provides additional in-editor functionality:
 * - automatic set the key name by specified tag.
 * - generate .json on asset change.
 * - reimport .json.
 */
UCLASS(BlueprintType)
class MYSETTINGSWIDGETCONSTRUCTOR_API USettingsDataTable : public UDataTable
{
	GENERATED_BODY()

public:
	/** Default constructor to set members as FSettingsRow. */
	USettingsDataTable();

	/** Returns the table rows.
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GenerateSettingsArray(TMap<FName, FSettingsPicker>& OutRows) const;
};
