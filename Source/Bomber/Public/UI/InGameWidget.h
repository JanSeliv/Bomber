// Copyright 2021 Yevhenii Selivanov.

#pragma once

#include "Bomber.h"
#include "Blueprint/UserWidget.h"
//---
#include "InGameWidget.generated.h"

/**
 * In game user widget.
 */
UCLASS(Abstract)
class BOMBER_API UInGameWidget final : public UUserWidget
{
	GENERATED_BODY()
public:
	/** The in game menu is shown the result of the games match (win, lose, draw).
	 *	If the match has not yet finished, it could be minimized or opened out by ESC button in order to
	 *	continue watching the game or restart the play, or to return to the main menu
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void ShowEndGameState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void HideEndGameState();

protected:
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

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
