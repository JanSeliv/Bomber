// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"
//---
#include "AssetTypeCategories.h"

class SETTINGSWIDGETCONSTRUCTOREDITOR_API FSettingsWidgetConstructorEditorModule : public IModuleInterface
{
public:
	/** Is used to load and unload the Property Editor Module. */
	inline static const FName PropertyEditorModule = TEXT("PropertyEditor");

	/** Is used to customize FSettingTag structure. */
	inline static const FName SettingTagStructureName = TEXT("SettingTag");

	/** Category of this plugin in the 'Add' context menu. */
	inline static EAssetTypeCategories::Type SettingsCategory = EAssetTypeCategories::None;

	/**
	 * Called right after the module DLL has been loaded and the module object has been created.
	 * Load dependent modules here, and they will be guaranteed to be available during ShutdownModule.
	 */
	virtual void StartupModule() override;

	/**
	* Called before the module is unloaded, right before the module object is destroyed.
	* During normal shutdown, this is called in reverse order that modules finish StartupModule().
	* This means that, as long as a module references dependent modules in it's StartupModule(), it
	* can safely reference those dependencies in ShutdownModule() as well.
	*/
	virtual void ShutdownModule() override;

protected:
	/** Creates all customizations for custom properties. */
	void RegisterPropertyCustomizations();

	/** Removes all custom property customizations. */
	void UnregisterPropertyCustomizations();

	/** Adds to context menu custom assets to be created. */
	void RegisterSettingAssets();

	/** Adds the category of this plugin to the 'Add' context menu. */
	void RegisterSettingsCategory();

	/** Asset type actions */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> RegisteredAssets;
};
