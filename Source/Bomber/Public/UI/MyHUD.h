// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"
//---
#include "MyHUD.generated.h"

/**
 * The custom HUD class. Also manages other widgets.
 * @see Access its data with UUIDataAsset (Content/Bomber/DataAssets/DA_UI).
 */
UCLASS(Config = "GameUserSettings")
class BOMBER_API AMyHUD final : public AHUD
{
	GENERATED_BODY()

public:
	/** ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetsInitialized);

	/** Is called to notify that all widgets were initialized and ready. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnWidgetsInitialized OnWidgetsInitialized;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClose);

	/** Is called to notify listen widgets to be closed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnClose OnClose;

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyHUD();

	/** Returns true if widgets ere initialized. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool AreWidgetInitialized() const { return bAreWidgetInitializedInternal; }

	/** Returns the current in-game widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UInGameWidget* GetInGameWidget() const { return InGameWidgetInternal; }

	/** Returns the current Main Menu widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UMainMenuWidget* GetMainMenuWidget() const { return MainMenuWidgetInternal; }

	/** Returns the current settings widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class USettingsWidget* GetSettingsWidget() const { return SettingsWidgetInternal; }

	/** Returns the current FPS counter widget object. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UUserWidget* GetFPSCounterWidget() const { return FPSCounterWidgetInternal; }

	/** Returns the nickname widget by a player index. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UUserWidget* GetNicknameWidget(int32 Index) const { return NicknameWidgetsInternal.IsValidIndex(Index) ? NicknameWidgetsInternal[Index] : nullptr; }

	/** Notify listen UI widgets to
	close widget. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void BroadcastOnClose();

	/** Set true to show the FPS counter widget on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSCounterEnabled(bool bEnable);

	/** Returns true if the FPS counter widget is shown on the HUD. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFPSCounterEnabled() const { return bIsFPSCounterEnabledInternal; }

	/** Add new Character Selection Spot, so it will be reflected in Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void AddCharacterSelectionSpot(class ACharacterSelectionSpot* CharacterSelectionSpot);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Is true if widgets are initialized. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Are Widget Initialized"))
	bool bAreWidgetInitializedInternal = false;

	/** The current in-game widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "In-Game Widget"))
	TObjectPtr<class UInGameWidget> InGameWidgetInternal = nullptr;

	/** The current Main Menu widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Main Menu Widget"))
	TObjectPtr<class UMainMenuWidget> MainMenuWidgetInternal = nullptr;

	/** The current settings widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<class USettingsWidget> SettingsWidgetInternal = nullptr;

	/** The current FPS counter widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Counter Widget"))
	TObjectPtr<class UUserWidget> FPSCounterWidgetInternal = nullptr;

	/** All nickname widget objects for each player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nickname Widgets"))
	TArray<TObjectPtr<class UUserWidget>> NicknameWidgetsInternal;

	/** If true, shows FPS counter widget on the HUD, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is FPS Counter Enabled"))
	bool bIsFPSCounterEnabledInternal;

	/** All Main Menu spots with characters placed on the level. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Character Selection Spots"))
	TArray<TObjectPtr<class ACharacterSelectionSpot>> CharacterSelectionSpotsInternal;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Init all widgets on gameplay starting before begin play. */
	virtual void PostInitializeComponents() override;

	/** Internal UUserWidget::CreateWidget wrapper. */
	static UUserWidget* CreateWidgetByClass(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, bool bAddToViewport = true);

	template <typename T = UUserWidget>
	FORCEINLINE T* CreateWidgetByClass(TSubclassOf<T> WidgetClass, bool bAddToViewport = true) const { return Cast<T>(CreateWidgetByClass(PlayerOwner.Get(), WidgetClass, bAddToViewport)); }

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TryInitWidgets();

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();

	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);
};
