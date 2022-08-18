// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AssetTypeActions/AssetTypeActions_MyDataTable.h"
//---
#include "AssetTypeActions_SettingsDataTable.generated.h"

/**
 * @todo
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
 * @todo
 */
class MYSETTINGSWIDGETCONSTRUCTOREDITOR_API FAssetTypeActions_SettingsDataTable : public FAssetTypeActions_MyDataTable
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
