// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "GameFramework/HUD.h"
//---
#include "MyHUD.generated.h"

/**
 * Contains in-game UI data.
 */
UCLASS(Blueprintable, BlueprintType)
class UUIDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the UI data asset. */
	static const UUIDataAsset& Get();

	/** Returns a class of the in-game widget.
	 * @see UUIDataAsset::InGameWidgetClassInternal.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UInGameWidget> GetInGameWidgetClass() const { return InGameWidgetClassInternal; }

	/** Returns a class of the settings widget.
	 * @see UUIDataAsset::SettingsWidgetClassInternal.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class USettingsWidget> GetSettingsWidgetClass() const { return SettingsWidgetClassInternal; }

	/** Returns a class of the nickname widget.
	 * @see UUIDataAsset::NicknameWidgetClassInternal.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetNicknameWidgetClass() const { return NicknameWidgetClassInternal; }

	/** Returns a class of the FPS counter widget.
	 * @see UUIDataAsset::FPSCounterWidgetClassInternal.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class UUserWidget> GetFPSCounterWidgetClass() const { return FPSCounterWidgetClassInternal; }

protected:
	/** The class of a In-Game Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "In-Game Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UInGameWidget> InGameWidgetClassInternal; //[D]

	/** The class of a Settings Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Settings Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingsWidget> SettingsWidgetClassInternal; //[D]

	/** The class of a Nickname Widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Nickname Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> NicknameWidgetClassInternal; //[D]

	/** The class of a FPS counter widget blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "FPS Counter Widget Class", ShowOnlyInnerProperties))
	TSubclassOf<class UUserWidget> FPSCounterWidgetClassInternal; //[D]
};

/**
 * The custom HUD class. Also manages other widgets.
 */
UCLASS(Config = "GameUserSettings")
class AMyHUD final : public AHUD
{
	GENERATED_BODY()

public:
	/** ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGoUIBack);

	/** Is called to notify widgets to go back. */
	UPROPERTY(BlueprintAssignable, Category = "C++")
	FOnGoUIBack OnGoUIBack;

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyHUD();

	/** Returns the current in-game widget object. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UInGameWidget* GetInGameWidget() const { return InGameWidgetInternal; }

	/** Returns the current settings widget object. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class USettingsWidget* GetSettingsWidget() const { return SettingsWidgetInternal; }

	/** Returns the current FPS counter widget object. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UUserWidget* GetFPSCounterWidget() const { return FPSCounterWidgetInternal; }

	/** Returns the nickname widget by a player index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UUserWidget* GetNicknameWidget(int32 Index) const { return NicknameWidgetsInternal.IsValidIndex(Index) ? NicknameWidgetsInternal[Index] : nullptr; }

	/** Go back input for UI widgets. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void GoUIBack();

	/** Set true to show the FPS counter widget on the HUD. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetFPSCounterEnabled(bool bEnable);

	/** Returns true if the FPS counter widget is shown on the HUD. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsFPSCounterEnabled() const { return bIsFPSCounterEnabledInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** The current in-game widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "In-Game Widget"))
	class UInGameWidget* InGameWidgetInternal; //[G]

	/** The current settings widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	class USettingsWidget* SettingsWidgetInternal; //[G]

	/** The current FPS counter widget object. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "FPS Counter Widget"))
	class UUserWidget* FPSCounterWidgetInternal; //[G]

	/** All nickname widget objects for each player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nickname Widgets"))
	TArray<class UUserWidget*> NicknameWidgetsInternal; //[G]

	/** If true, shows FPS counter widget on the HUD. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "C++", meta = (BlueprintProtected, DisplayName = "Is FPS Counter Enabled"))
	bool bIsFPSCounterEnabledInternal; //[С]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Init all widgets on gameplay starting before begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;

	/** Create and set widget objects once. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitWidgets();
};
