// Copyright 2019 Yevhenii Selivanov.

#pragma once

#include "Blueprint/UserWidget.h"

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
	 *	@param DisplayedWidget The widget that will be shown.
	 *	@todo Rewrite to C++
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++")
	void ShowInGameState(const class UWidget* DisplayedWidget = nullptr);

protected:
	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	void NativeConstruct() override;

	/* Updates appearance dynamically in the editor. */
	virtual void SynchronizeProperties() override;
};
