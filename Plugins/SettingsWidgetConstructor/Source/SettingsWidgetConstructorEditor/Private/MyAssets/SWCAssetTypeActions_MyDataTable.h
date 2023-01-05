// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "AssetTypeActions_CSVAssetBase.h"
#include "Factories/ReimportDataTableFactory.h"
//---
#include "SWCAssetTypeActions_MyDataTable.generated.h"

/**
 * Is responsible for creating new custom Data Table asset in the 'Add' context menu.
 * Is abstract to ignore this factory by !CurrentClass->HasAnyClassFlags(CLASS_Abstract),
 * child should not be abstracted
 */
UCLASS(Abstract)
class USWCMyDataTableFactory : public UReimportDataTableFactory
{
	GENERATED_BODY()

public:
#pragma region OverrideInChild
	USWCMyDataTableFactory();
	virtual FText GetDisplayName() const override;
#pragma endregion OverrideInChild

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ConfigureProperties() override { return true; }
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual bool DoesSupportClass(UClass* Class) override;
};

/**
 * Shows additional actions in the Context Menu of the custom data table assets.
 * Basically, is taken from FAssetTypeActions_DataTable since we can't derive to private include,
 * but need its functionality like import and exporting .json files.
 */
class FAssetTypeActions_MyDataTable : public FAssetTypeActions_CSVAssetBase
{
public:
	virtual ~FAssetTypeActions_MyDataTable() override = default;

#pragma region OverrideInChild
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return FColor::Blue; }
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
#pragma endregion OverrideInChild

#pragma region FAssetTypeActions_DataTable
	/** Shows 'Export as JSON' option in the context menu. */
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;

	/** Is overridden to show 'reimport' options in the contexts menu. */
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;

	/** Opens data within the data table editor. */
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	/** Low-level .json exporter, is called when 'Export as JSON' actions was pressed. */
	virtual void ExecuteExportAsJSON(TArray<TWeakObjectPtr<UObject>> Objects);
#pragma endregion FAssetTypeActions_DataTable
};
