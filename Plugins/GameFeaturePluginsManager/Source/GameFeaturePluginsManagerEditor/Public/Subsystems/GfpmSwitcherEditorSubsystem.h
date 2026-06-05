// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EditorSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "GfpmSwitcherEditorSubsystem.generated.h"

/**
 * Manages Swicher widget editor tab.
 * Docks next to Scene Outliner, by default, open on startup.
 */
UCLASS(Config = "GameFeaturePluginsManager", DefaultConfig, DisplayName = "Game Feature Plugins Switcher Editor Subsystem")
class GAMEFEATUREPLUGINSMANAGEREDITOR_API UGfpmSwitcherEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** Opens or focuses switcher tab in its docked spot, also action behind Window menu entry. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]")
	void OpenSwitcherTab();

	/** Closes switcher tab if currently open. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]")
	void CloseSwitcherTab();

	/** Returns true if switcher tab is currently open in level-editor tab manager. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager Editor]")
	bool IsSwitcherTabOpen() const;

protected:
	/** Switcher Widget Blueprint hosted in editor tab, config-backed so any project can override it. */
	UPROPERTY(Config, VisibleInstanceOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	TSoftClassPtr<class UUserWidget> SwitcherWidgetClass;

	/** Editor switcher instance kept alive while its tab is open, distinct from any runtime-world instance. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Transient, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	TObjectPtr<class UUserWidget> SwitcherWidget = nullptr;

	/** Level-editor layout-extension subscription handle, docks tab next to Scene Outliner. */
	FDelegateHandle LayoutExtensionHandle;

	/** Level-editor register-tabs subscription handle, registers spawner on its tab manager when layout is built. */
	FDelegateHandle RegisterTabsHandle;

	/** Blueprint-reinstanced subscription handle, refreshes tab content after switcher Widget Blueprint recompiles. */
	FDelegateHandle BlueprintReinstancedHandle;

	/** Registers switcher tab spawner on level-editor tab manager, no-op if already registered or unavailable. */
	void RegisterTabSpawner(TSharedPtr<class FTabManager> InTabManager);

	/** Returns level-editor tab manager that owns docked switcher tab, nullptr when unavailable. */
	TSharedPtr<FTabManager> GetSwitcherTabManager() const;

	/** Finds live switcher tab in level-editor tab manager, nullptr when not open. */
	TSharedPtr<class SDockTab> FindSwitcherTab() const;

	/** When tab manager needs to spawn switcher dockable tab.
	 * Non-UFUNCTION exception, TSharedRef<SDockTab> return type is not Blueprint-reflectable. */
	TSharedRef<SDockTab> OnSpawnTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** Creates switcher widget on editor world and returns its Slate content for tab. */
	TSharedRef<class SWidget> MakeSwitcherTabContent();

	/** When Blueprint asset is reinstanced in editor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Game Feature Plugins Manager Editor]", meta = (BlueprintProtected))
	void OnBlueprintReinstanced();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** When subsystem initializes. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** When subsystem is destroyed. */
	virtual void Deinitialize() override;
};
