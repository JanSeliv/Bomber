// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Modules/ModuleInterface.h"
//---
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"

class SETTINGSWIDGETCONSTRUCTOREDITOR_API FSettingsWidgetConstructorEditorModule : public IModuleInterface
{
public:
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

#pragma region EditorExtensions
	/** Adds the asset to the context menu
	 * @param InOutRegisteredAssets Input: the list of registered assets. Output: the list of registered assets + the new one. */
	template <typename T>
	static void RegisterAsset(TArray<TSharedPtr<FAssetTypeActions_Base>>& InOutRegisteredAssets);

	/** Removes all custom assets from context menu. */
	static void UnregisterAssets(TArray<TSharedPtr<FAssetTypeActions_Base>>& RegisteredAssets);
#pragma endregion EditorExtensions

protected:
	/** Creates all customizations for custom properties. */
	static void RegisterPropertyCustomizations();

	/** Removes all custom property customizations. */
	static void UnregisterPropertyCustomizations();

	/** Adds to context menu custom assets to be created. */
	void RegisterSettingAssets();

	/** Adds the category of this plugin to the 'Add' context menu. */
	static void RegisterSettingAssetsCategory();

	/** Asset type actions */
	TArray<TSharedPtr<FAssetTypeActions_Base>> RegisteredAssets;
};

// Adds the asset to the context menu
template <typename T>
void FSettingsWidgetConstructorEditorModule::RegisterAsset(TArray<TSharedPtr<FAssetTypeActions_Base>>& InOutRegisteredAssets)
{
	TSharedPtr<T> SettingsDataTableAction = MakeShared<T>();
	IAssetTools::Get().RegisterAssetTypeActions(SettingsDataTableAction.ToSharedRef());
	InOutRegisteredAssets.Emplace(MoveTemp(SettingsDataTableAction));
}
