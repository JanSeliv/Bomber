// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "InGameWidget.generated.h"

enum class ECurrentGameState : uint8;

/**
 * In game user widget.
 */
UCLASS()
class BOMBER_API UInGameWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Returns the In-Game Menu widget. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UInGameMenuWidget* GetInGameMenuWidget() const { return InGameMenuWidget; }

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Allows player to interact with UI during the match. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UInGameMenuWidget> InGameMenuWidget = nullptr;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/**
	 * Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy.
	 */
	virtual void NativeConstruct() override;

	/** Launch 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void LaunchStartingCountdown();

	/** Launch the main timer that count the seconds to the game ending. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void LaunchInGameCountdown();

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Is called to start listening game state changes. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void BindOnGameStateChanged(class AMyGameStateBase* MyGameState);
};
