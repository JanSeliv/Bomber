// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

// UE
#include "GameFeatureStateChangeObserver.h"
#include "Templates/SubclassOf.h"

#include "GfpmSwitcherWidget.generated.h"

/**
 * Self-contained switcher UI that lists every registered Game Feature Plugin (GFP) with on/off toggle.
 * Serves players in runtime worlds and developers in editor-not-PIE world (GFPM editor tab).
 */
UCLASS(Abstract, DisplayName = "Game Feature Plugins Switcher Widget")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmSwitcherWidget : public UUserWidget
    , public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	/** Returns switcher added to viewport of given context's play world, nullptr if none.
	 * Serves as Settings owner-context getter for Mods button that opens this widget.
	 * @param WorldContext Any object whose play world hosts switcher. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Game Feature Plugins Manager]", meta = (WorldContext = "WorldContext"))
	static UGfpmSwitcherWidget* GetRuntimeSwitcherWidget(const UObject* WorldContext);

	/** When Mods settings button is pressed. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Game Feature Plugins Manager]")
	void OnSwitcherOpened();

	/** Collapses switcher back, bound to in-widget Close button. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void CloseSwitcher();

	/** Flip-flops switcher visibility, used by debug cheats to drive it without settings menu. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void ToggleSwitcher();

	/** Presents switcher as always-open embedded panel where host (editor tab) owns closing. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void ShowInEditorTab();

	/*********************************************************************************************
	 * Widgets
	 ********************************************************************************************* */
protected:
	/** Container that hosts one toggle row per registered Game Feature Plugin. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UPanelWidget> PluginsList = nullptr;

	/** Collapses switcher when pressed, hidden in editor-not-PIE world where host tab owns closing. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> CloseButton = nullptr;

	/** Designer-assigned row widget spawned per plugin, all its styling lives in its own Widget Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	TSubclassOf<class UGfpmSwitcherRowWidget> RowWidgetClass = nullptr;

	/*********************************************************************************************
	 * Logic
	 ********************************************************************************************* */
protected:
	/** Rebuilds rows from all registered Game Feature Plugins. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void RebuildRows();

	/** Tells every row to re-read its plugin's active state. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void RefreshRowStates();

	/** When Close button is pressed. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnCloseButtonPressed();

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** When widget is constructed and added to viewport. Not exposed: parent virtual override parent didn't expose. */
	virtual void NativeConstruct() override;

	/** When widget is destroyed and removed from viewport. Not exposed: parent virtual override parent didn't expose. */
	virtual void NativeDestruct() override;

	/** When owning game feature plugin starts activating. Not exposed: parent virtual override parent didn't expose. */
	virtual void OnGameFeatureActivating(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;

	/** When owning game feature plugin starts deactivating. Not exposed: parent virtual override parent didn't expose. */
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context, const FString& PluginURL) override;
};
