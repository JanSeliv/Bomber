// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Blueprint/UserWidget.h"
//---
#include "InGameWidget.generated.h"

/**
 * In game user widget.
 */
UCLASS()
class UInGameWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggledInGameMenu, bool, bIsVisible);

	/** Is called to notify listeners is opened In-Game Menu. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnToggledInGameMenu OnToggledInGameMenu;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Checks the visibilities of in-game menu subwidgets and returns true if are visible */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent, Category = "C++")
	bool IsVisibleInGameMenu() const;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The button to allow player restart the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> RestartButton = nullptr; //[B]

	/** The button to allow player go back to the Main Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> MenuButton = nullptr; //[B]

	/** The button to allow player open in-game Settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> SettingsButton = nullptr; //[B]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;

	/** Launch 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void LaunchStartingCountdown();

	/** Launch the main timer that count the seconds to the game ending. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void LaunchInGameCountdown();

	/** The in game menu is shown the result of the games match (win, lose, draw).
	*	If the match has not yet finished, it could be minimized or opened out by ESC button in order to
	*	continue watching the game or restart the play, or to return to the main menu. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void ShowInGameMenu();

	/** Hide the in game menu. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void HideInGameMenu();

	/** Flip-floppy show and hide the end in-game menu subwidgets. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ToggleInGameMenu();

	/** Called when the visibility of this widgets was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnWidgetVisibilityChanged(ESlateVisibility InVisibility);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Is called when player pressed the button to restart the game. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnRestartButtonPressed();

	/** Is called when player pressed the button to go back to the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnMenuButtonPressed();

	/** Is called when player pressed the button to open in-game Settings. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnSettingsButtonPressed();
};
