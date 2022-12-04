// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Components/WidgetInteractionComponent.h"
//---
#include "Bomber.h"
//---
#include "MenuWidgetInteractionComponent.generated.h"

/**
 * Custom Widget Interaction Component.
 * used to handle inputs in the Main Menu 3D widget placed a in world.
 * @see UMainMenuWidget
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOMBER_API UMenuWidgetInteractionComponent final : public UWidgetInteractionComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UMenuWidgetInteractionComponent();

	/** Sends the press key event to slate, is overriden to ignore if component is not active. */
	virtual bool PressKey(FKey Key, bool bRepeat) override;

	/** Sends the release key event to slate, is overriden to ignore if component is not active. */
	virtual bool ReleaseKey(FKey Key) override;

	/** Sets most suitable Virtual User index by current player index. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdatePlayerIndex();

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/**
	 * Presses a key as if the mouse/pointer were the source of it.  Normally you would just use
	 * Left/Right mouse button for the Key.  However - advanced uses could also be imagined where you
	 * send other keys to signal widgets to take special actions if they're under the cursor.
	 */
	virtual void PressPointerKey(FKey Key) override;

	/** Listen game states to manage the enabling and disabling this component. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Pushes the owner actor to the stack of input to be able to send input key events. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void EnableInput();

	/* ---------------------------------------------------
	 *		Listen Settings
	 * --------------------------------------------------- */

	/** Binds to toggle Settings to be able enable or disable this component. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnToggledSettings();

	/** Is called when all widgets are initialized to bind on settings toggle. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/** Disables this component while setting are opened and vice versa. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnToggledSettings(bool bIsVisible);
};
