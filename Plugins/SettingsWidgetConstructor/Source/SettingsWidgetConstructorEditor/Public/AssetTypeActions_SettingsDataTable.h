// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MyAssets/SWCAssetTypeActions_MyDataTable.h"
//---
#include "AssetTypeActions_SettingsDataTable.generated.h"

/**
 * Is responsible for creating new Settings Data Table asset in the 'Add' context menu.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOREDITOR_API USettingsDataTableFactory : public USWCMyDataTableFactory
{
	GENERATED_BODY()

public:
	USettingsDataTableFactory();
	virtual FText GetDisplayName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

protected:
	/** Imports default data into new Settings Data Table. */
	virtual void ImportDefaultSettingsDataTable(UObject* NewSettingDataTable);
};

/**
 * Shows additional actions in the Context Menu.
 */
class SETTINGSWIDGETCONSTRUCTOREDITOR_API FAssetTypeActions_SettingsDataTable : public FAssetTypeActions_MyDataTable
{
public:
	virtual ~FAssetTypeActions_SettingsDataTable() override = default;

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
