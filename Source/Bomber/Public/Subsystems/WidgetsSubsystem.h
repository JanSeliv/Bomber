// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
//---
#include "WidgetsSubsystem.generated.h"

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

	/** ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetsInitialized);

	/** Is called to notify that all widgets were initialized and ready. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnWidgetsInitialized OnWidgetsInitialized;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClose);

	/** Is called to notify listen widgets to be closed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnClose OnClose;

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Returns true if widgets ere initialized. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool AreWidgetInitialized() const { return bAreWidgetInitializedInternal; }

	/** Returns the current in-game widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UHUDWidget* GetHUDWidget() const { return HUDWidgetInternal; }

	/** Returns the current settings widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class USettingsWidget* GetSettingsWidget() const { return SettingsWidgetInternal; }

	/** Returns the current FPS counter widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UUserWidget* GetFPSCounterWidget() const { return FPSCounterWidgetInternal; }

	/** Returns the nickname widget by a player index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UPlayerName3DWidget* GetNicknameWidget(int32 Index) const { return NicknameWidgetsInternal.IsValidIndex(Index) ? NicknameWidgetsInternal[Index] : nullptr; }

	/** Set true to show the FPS counter widget on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSCounterEnabled(bool bEnable);

	/** Returns true if the FPS counter widget is shown on the HUD. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFPSCounterEnabled() const { return bIsFPSCounterEnabledInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Is true if widgets are initialized. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Are Widget Initialized"))
	bool bAreWidgetInitializedInternal = false;

	/** The current in-game widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "HUD Widget"))
	TObjectPtr<class UHUDWidget> HUDWidgetInternal = nullptr;

	/** The current settings widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<class USettingsWidget> SettingsWidgetInternal = nullptr;

	/** The current FPS counter widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Counter Widget"))
	TObjectPtr<class UUserWidget> FPSCounterWidgetInternal = nullptr;

	/** All nickname widget objects for each player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nickname Widgets"))
	TArray<TObjectPtr<class UPlayerName3DWidget>> NicknameWidgetsInternal;

	/** If true, shows FPS counter widget on the HUD, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is FPS Counter Enabled"))
	bool bIsFPSCounterEnabledInternal;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Callback for when the player controller is changed on this subsystem's owning local player. */
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TryInitWidgets();

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();

	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);
};