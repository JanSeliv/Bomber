// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "InGameMenuWidget.generated.h"

/**
 * Allows player to interact with UI during the match.
 * Is shown automatically on ending the game.
 * It could be opened by ESC button during the game.
 */
UCLASS()
class BOMBER_API UInGameMenuWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggledInGameMenu, bool, bIsVisible);

	/** Is called to notify listeners the In-Game Menu is opened or closed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnToggledInGameMenu OnToggledInGameMenu;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Returns true if the In-Game widget is shown on user screen. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsVisibleInGameMenu() const;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** The button to allow player restart the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> RestartButton = nullptr;

	/** The button to allow player go back to the Main Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> MenuButton = nullptr;

	/** The button to allow player open in-game Settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> SettingsButton = nullptr;

	/** Contains the localized text with the match result when the match was ended. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> EndGameTextWidget = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Called when the end-game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EEndGameState EndGameState);

	/** Is called to start listening game state changes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnGameStateChanged(class AMyGameStateBase* MyGameState);

	/** Is called to start listening End-Game state changes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnEndGameStateChanged(class AMyPlayerState* MyPlayerState);

	/** Is called when player pressed the button to restart the game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnRestartButtonPressed();

	/** Is called when player pressed the button to go back to the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMenuButtonPressed();

	/** Is called when player pressed the button to open in-game Settings. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnSettingsButtonPressed();

	/** Called to set Win,Draw or lose text on UI. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateEndGameText();

	/** The in game menu is shown the result of the games match (win, lose, draw).
	 *	If the match has not yet finished, it could be minimized or opened out by ESC button in order to
	 *	continue watching the game or restart the play, or to return to the main menu. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ShowInGameMenu();

	/** Hide the in game menu. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void HideInGameMenu();

	/** Flip-floppy show and hide the end in-game menu subwidgets. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ToggleInGameMenu();

	/** Is called when In-Game menu became opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggleInGameMenu(bool bIsVisible);
};
