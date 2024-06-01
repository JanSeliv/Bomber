// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "HUDWidget.generated.h"

enum class EEndGameState : uint8;

/**
 * Is displayed on the screen during the match.
 * All sub-widget properties are created in bp widget asset and managed with View Models.
 */
UCLASS(Abstract)
class BOMBER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Widget animation to show the end-game result (win/lose/draw). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> ResultAnimation = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when the local player state is initialized and its assigned character is ready. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalPlayerStateReady(class AMyPlayerState* PlayerState, int32 CharacterID);

	/** Is called on end-game result change. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EEndGameState EndGameState);
};