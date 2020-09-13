// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Bomber.h"
//---
#include "InGameWidget.generated.h"

/**
 * In game user widget.
 */
UCLASS(Abstract)
class BOMBER_API UInGameWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** The in game menu is shown the result of the games match (win, lose, draw).
	 *	If the match has not yet finished, it could be minimized or opened out by ESC button in order to
	 *	continue watching the game or restart the play, or to return to the main menu:
	 *	@param DisplayedWidget The widget that will be shown on upper (nullptr for hide menu).
	 *	@TODO Rewrite to C++
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	void ShowInGameState(const class UWidget* DisplayedWidget = nullptr);

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

	/** */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
    void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
