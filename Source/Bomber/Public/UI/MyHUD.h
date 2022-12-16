// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"
//---
#include "Bomber.h"
//---
#include "MyHUD.generated.h"

/**
 * Contains in-game UI data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UUIDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UUIDataAsset& Get();

	/** Returns a class of the in-game widget.
	 * @see UUIDataAsset::InGameWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UInGameWidget> GetInGameWidgetClass() const { return InGameWidgetClassInternal; }

	/** Returns a class of the main menu widget.
	 * @see UUIDataAsset::MainMenuWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UMainMenuWidget> GetMainMenuWidgetClass() const { return MainMenuWidgetClassInternal; }

	/** Returns a class of the settings widget.
	 * @see UUIDataAsset::SettingsWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingsWidget> GetSettingsWidgetClass() const { return SettingsWidgetClassInternal; }

	/** Returns a class of the nickname widget.
	 * @see UUIDataAsset::NicknameWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetNicknameWidgetClass() const { return NicknameWidgetClassInternal; }

	/** Returns a class of the FPS counter widget.
	 * @see UUIDataAsset::FPSCounterWidgetClassInternal.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetFPSCounterWidgetClass() const { return FPSCounterWidgetClassInternal; }

	/** Returns the localized texts about specified end game to display on UI. 
	 * @see UUIDataAsset::EndGameTexts. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FText& GetEndGameText(EEndGameState EndGameState) const { return EndGameTextsInternal.FindChecked(EndGameState); }

protected:
	/** The class of a In-Game Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UInGameWidget> InGameWidgetClassInternal = nullptr;

	/** The class of a In-Game Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Main Menu Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UMainMenuWidget> MainMenuWidgetClassInternal = nullptr;

	/** The class of a Settings Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingsWidget> SettingsWidgetClassInternal = nullptr;

	/** The class of a Nickname Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Nickname Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> NicknameWidgetClassInternal = nullptr;

	/** The class of a FPS counter widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "FPS Counter Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> FPSCounterWidgetClassInternal = nullptr;

	/** Contains the localized texts about specified end game to display on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "End-Game Texts", ShowOnlyInnerProperties))
	TMap<EEndGameState, FText> EndGameTextsInternal;
};

/**
 * The custom HUD class. Also manages other widgets.
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

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Init all widgets on gameplay starting before begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;

	/** Will try to start the process of initializing all widgets used in game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TryInitWidgets();

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();

	/** Is called right after the game was started and windows size is set. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);
};
