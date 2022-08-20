// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MyDataTable//AssetTypeActions_MyDataTable.h"
//---
#include "AssetTypeActions_SettingsDataTable.generated.h"

/**
 * Is responsible for creating new Settings Data Table asset in the 'Add' context menu.
 */
UCLASS()
class MYSETTINGSWIDGETCONSTRUCTOREDITOR_API USettingsDataTableFactory : public UMyDataTableFactory
{
	GENERATED_BODY()

public:
	USettingsDataTableFactory();
	virtual FText GetDisplayName() const override;
};

/**
 * Shows additional actions in the Context Menu.
 */
class MYSETTINGSWIDGETCONSTRUCTOREDITOR_API FAssetTypeActions_SettingsDataTable : public FAssetTypeActions_MyDataTable
{
public:
	virtual ~FAssetTypeActions_SettingsDataTable() override = default;

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
