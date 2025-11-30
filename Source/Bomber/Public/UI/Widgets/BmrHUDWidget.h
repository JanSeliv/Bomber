// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"

#include "BmrHUDWidget.generated.h"

enum class EBmrEndGameState : uint8;

/**
 * Is displayed on the screen during the match.
 * All sub-widget properties are created in bp widget asset and managed with View Models.
 */
UCLASS(Abstract)
class BOMBER_API UBmrHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Widget animation to show the end-game result (win/lose/draw). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "[Bomber]", meta = (BlueprintProtected, BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> ResultAnimation = nullptr;

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
};