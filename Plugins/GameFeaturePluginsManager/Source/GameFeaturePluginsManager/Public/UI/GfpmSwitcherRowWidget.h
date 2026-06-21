// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "GfpmSwitcherRowWidget.generated.h"

/**
 * Single row in Game Feature Plugins switcher, shows one plugin name with on/off toggle.
 */
UCLASS(Abstract, DisplayName = "Game Feature Plugins Switcher Row Widget")
class GAMEFEATUREPLUGINSMANAGER_API UGfpmSwitcherRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Binds this row to Game Feature Plugin and shows its current active state.
	 * @param InGameFeatureName Name of plugin this row toggles. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void InitRow(FName InGameFeatureName);

	/** Re-reads plugin active state into toggle, used when another switcher changed it. */
	UFUNCTION(BlueprintCallable, Category = "[Game Feature Plugins Manager]")
	void RefreshActiveState();

	/*********************************************************************************************
	 * Package events
	 ********************************************************************************************* */
public:
	/** Fired when this row Package button is pressed. Not exposed: editor-only package event bound in C++. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FGfpmOnRowPackageRequested, FName /*GameFeatureName*/);
	FGfpmOnRowPackageRequested OnPackageRequested;

	/*********************************************************************************************
	 * Widgets
	 ********************************************************************************************* */
protected:
	/** Shows plugin name. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> PluginNameText = nullptr;

	/** Toggles plugin on or off. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UCheckBox> ActiveCheckBox = nullptr;

	/** Packages mod, triggers directory picker: installs into packaged build or extracts standalone. Shown only in editor-not-PIE world. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected, BindWidgetOptional, DevelopmentOnly))
	TObjectPtr<class UButton> PackageButton = nullptr;

	/** Plugin this row toggles. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	FName GameFeatureName = NAME_None;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** When active check state changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnActiveCheckStateChanged(bool bIsChecked);

	/** When Package button is pressed. */
	UFUNCTION(BlueprintNativeEvent, Category = "[Game Feature Plugins Manager]", meta = (BlueprintProtected))
	void OnPackageButtonPressed();

	/** When this widget is constructed. Not exposed: parent virtual override parent didn't expose. */
	virtual void NativeConstruct() override;

	/** When this widget is destroyed. Not exposed: parent virtual override parent didn't expose. */
	virtual void NativeDestruct() override;
};
