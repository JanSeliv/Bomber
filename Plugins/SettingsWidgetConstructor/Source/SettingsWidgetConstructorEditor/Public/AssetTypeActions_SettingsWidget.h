// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MyAssets/SWCAssetTypeActions_MyUserWidget.h"
//---
#include "AssetTypeActions_SettingsWidget.generated.h"

/**
 * Is responsible for creating new Settings Widget in the 'Add' context menu.
 */
UCLASS(Config = SettingsWidgetConstructor, DefaultConfig)
class SETTINGSWIDGETCONSTRUCTOREDITOR_API USettingsWidgetFactory : public USWCMyUserWidgetFactory
{
	GENERATED_BODY()

public:
	USettingsWidgetFactory();

	virtual FText GetDisplayName() const override;
	virtual TSubclassOf<class UUserWidget> GetWidgetClass() const override;

private:
	/** The blueprint parent class is required for widget creation, if is not set in config, then USettingsWidget will be used.
	 * @TODO Remove this property after USettingsWidget blueprint will be completely moved to code. */
	UPROPERTY(Config)
	TSubclassOf<class USettingsWidget> SettingsWidgetClassInternal = nullptr;
};

/**
 * Shows additional actions in the Context Menu.
 */
class SETTINGSWIDGETCONSTRUCTOREDITOR_API FAssetTypeActions_SettingsWidget : public FAssetTypeActions_MyUserWidget
{
public:
	virtual ~FAssetTypeActions_SettingsWidget() override = default;

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
};
