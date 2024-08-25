// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
//---
#include "WidgetsSubsystem.generated.h"

class UUserWidget;

/**
 * Is used to manage User Widgets with lifetime of Local Player (similar to HUD).
 * @see Access its data with UUIDataAsset (Content/Bomber/DataAssets/DA_UI).
 */
UCLASS(Config = "GameUserSettings", DefaultConfig)
class BOMBER_API UWidgetsSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Returns the pointer the UI Subsystem.
	 * It will return null if Local Player is not initialized yet. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (WorldContext = "OptionalWorldContext", CallableWithoutWorldContext))
	static UWidgetsSubsystem* GetWidgetsSubsystem(const UObject* OptionalWorldContext = nullptr);

	/*********************************************************************************************
	 * Widgets Creation
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetsInitialized);

	/** Is called to notify that all widgets were initialized and ready. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnWidgetsInitialized OnWidgetsInitialized;

	/** Create specified widget and add it to Manageable widgets list, so its visibility can be changed globally. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	UUserWidget* CreateManageableWidget(TSubclassOf<UUserWidget> WidgetClass, bool bAddToViewport = true, int32 ZOrder = 0, const UObject* OptionalWorldContext = nullptr);

	template <typename T = UUserWidget>
	FORCEINLINE T* CreateManageableWidgetChecked(TSubclassOf<T> WidgetClass, bool bAddToViewport = true, int32 ZOrder = 0) { return CastChecked<T>(CreateManageableWidget(WidgetClass, bAddToViewport, ZOrder)); }

	/** Removes given widget from the list and destroys it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void DestroyManageableWidget(UUserWidget* Widget);

	/** Returns true if widgets ere initialized. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool AreWidgetInitialized() const { return bAreWidgetInitializedInternal; }

protected:
	/** Is true if widgets are initialized. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Are Widget Initialized"))
	bool bAreWidgetInitializedInternal = false;

	/** Contains all widgets that are managed by this subsystem. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "All Managable Widgets"))
	TArray<TObjectPtr<UUserWidget>> AllManageableWidgetsInternal;

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryInitWidgets();

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();

	/** Removes all widgets and transient data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void CleanupWidgets();

	/*********************************************************************************************
	 * Cached Widgets
	 ********************************************************************************************* */
public:
	/** Returns the current in-game widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UHUDWidget* GetHUDWidget() const { return HUDWidgetInternal; }

	/** Returns the current settings widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class USettingsWidget* GetSettingsWidget() const { return SettingsWidgetInternal; }

	/** Returns the nickname widget by a player index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UPlayerName3DWidget* GetNicknameWidget(int32 Index) const { return NicknameWidgetsInternal.IsValidIndex(Index) ? NicknameWidgetsInternal[Index] : nullptr; }

protected:
	/** The current in-game widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "HUD Widget"))
	TObjectPtr<class UHUDWidget> HUDWidgetInternal = nullptr;

	/** The current settings widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<class USettingsWidget> SettingsWidgetInternal = nullptr;

	/** All nickname widget objects for each player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nickname Widgets"))
	TArray<TObjectPtr<class UPlayerName3DWidget>> NicknameWidgetsInternal;

	/*********************************************************************************************
	 * FPS Counter
	 ********************************************************************************************* */
public:
	/** Returns the current FPS counter widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE UUserWidget* GetFPSCounterWidget() const { return FPSCounterWidgetInternal; }

	/** Set true to show the FPS counter widget on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSCounterEnabled(bool bEnable);

	/** Returns true if the FPS counter widget is shown on the HUD. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFPSCounterEnabled() const { return bIsFPSCounterEnabledInternal; }

protected:
	/** The current FPS counter widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Counter Widget"))
	TObjectPtr<UUserWidget> FPSCounterWidgetInternal = nullptr;

	/** If true, shows FPS counter widget on the HUD, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is FPS Counter Enabled"))
	bool bIsFPSCounterEnabledInternal;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:

	friend class UMyCheatManager;

	/** Callback for when the player controller is changed on this subsystem's owning local player. */
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	/** Is called when this Subsystem is removed. */
	virtual void Deinitialize() override;

	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);
};