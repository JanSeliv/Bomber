// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "BmrHUDWidget.generated.h"

enum class EBmrEndGameState : uint8;

class UButton;

/**
 * Is displayed on the screen during the match.
 * All sub-widget properties are created in bp widget asset and managed with View Models.
 */
UCLASS(Abstract)
class BOMBER_API UBmrHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Widget animation to show the end-game result (win/lose/draw). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> ResultAnimation = nullptr;

	/** The button to restart the current match. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> RestartButton = nullptr;

	/** The button to return to the Main Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> MenuButton = nullptr;

	/** The button to open the Settings Menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<UButton> SettingsButton = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the local player state is initialized and its assigned character is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnLocalPlayerStateReady(class ABmrPlayerState* PlayerState, int32 PlayerId);

	/** Is called on end-game result change. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EBmrEndGameState EndGameState);

	/** Is called when player pressed the button to restart the match. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnRestartButtonPressed();

	/** Is called when player pressed the button to return to the Main Menu. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnMenuButtonPressed();

	/** Is called when player pressed the button to open the Settings. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnSettingsButtonPressed();
};