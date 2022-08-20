// Copyright (c) Yevhenii Selivanov

#pragma once

#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
//---
#include "AssetTypeActions_SettingsDataAsset.generated.h"

/**
 * Is responsible for creating new Settings Data Asset in the 'Add' context menu.
 */
UCLASS()
class MYSETTINGSWIDGETCONSTRUCTOREDITOR_API USettingsDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	USettingsDataAssetFactory();
	virtual FText GetDisplayName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

class MYSETTINGSWIDGETCONSTRUCTOREDITOR_API FAssetTypeActions_SettingsDataAsset : public FAssetTypeActions_Base
{
public:
	virtual ~FAssetTypeActions_SettingsDataAsset() override = default;

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
