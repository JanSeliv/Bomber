// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"

// Bomber
#include "Bomber.h"

// UE
#include "Components/SlateWrapperTypes.h"
#include "Engine/TimerHandle.h"

#include "BmrMVVM_GameViewModel.generated.h"

/**
 * Contains general data to be used only by widgets.
 */
UCLASS(DisplayName = "Bomber Game View Model")
class BOMBER_API UBmrMVVM_GameViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Current Game State
	 ********************************************************************************************* */
public:
	/** Setter and Getter widgets about the current game state. */
	UFUNCTION()
	void SetCurrentGameState(EBmrCurrentGameState NewCurrentGameState) { UE_MVVM_SET_PROPERTY_VALUE(CurrentGameState, NewCurrentGameState); }

	EBmrCurrentGameState GetCurrentGameState() const { return CurrentGameState; }

protected:
	/** Represents the current game state.
	 * Is commonly used by 'UBmrBlueprintFunctionLibrary::GetVisibilityByGameState' to show or hide own widget. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	EBmrCurrentGameState CurrentGameState = EBmrCurrentGameState::None;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/*********************************************************************************************
	 * End-Game State
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the End-Game State visibility. */
	void SetEndGameStateVisibility(const ESlateVisibility& NewEndGameStateVisibility) { UE_MVVM_SET_PROPERTY_VALUE(EndGameStateVisibility, NewEndGameStateVisibility); }
	const ESlateVisibility& GetEndGameStateVisibility() const { return EndGameStateVisibility; }

	/** Setter and Getter about the End-Game result text. */
	void SetEndGameResult(const FText& NewEndGameResult) { UE_MVVM_SET_PROPERTY_VALUE(EndGameResult, NewEndGameResult); }
	const FText& GetEndGameResult() const { return EndGameResult; }

protected:
	/** Is 'Visible' when the game is ended with any result (win, lose, draw). */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	ESlateVisibility EndGameStateVisibility = ESlateVisibility::Collapsed;

	/** The result of the game: Win, Lose, Draw. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	FText EndGameResult = FText::GetEmpty();

	/** Called when the player state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EBmrEndGameState NewEndGameState);

	/*********************************************************************************************
	 * Countdown timers
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	void SetStartingTimerSecRemain(const FText& NewStartingTimerSecRemain) { UE_MVVM_SET_PROPERTY_VALUE(StartingTimerSecRemain, NewStartingTimerSecRemain); }
	const FText& GetStartingTimerSecRemain() const { return StartingTimerSecRemain; }

	/** Setter and Getter about the seconds to the end of the round. */
	void SetInGameTimerSecRemain(const FText& NewInGameTimerSecRemain) { UE_MVVM_SET_PROPERTY_VALUE(InGameTimerSecRemain, NewInGameTimerSecRemain); }
	const FText& GetInGameTimerSecRemain() const { return InGameTimerSecRemain; }

protected:
	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	FText StartingTimerSecRemain = FText::GetEmpty();

	/** Seconds to the end of the round. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	FText InGameTimerSecRemain = FText::GetEmpty();

	/** Handles time counting during the Game Starting state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	FTimerHandle StartingTimer;

	/** Starts counting the 3-2-1-GO timer when match is starting. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TriggerStartingCountdown();

	/** Clears the Starting timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void StopStartingCountdown();

	/** Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnStartingTimerTick();

	/** Handles time counting during the In-Game state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	FTimerHandle InGameTimer;

	/** Starts counting the (120...0) timer during the match. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void TriggerInGameCountdown();

	/** Clears the In-Game timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void StopInGameCountdown();

	/** Is called once a second during the In-Game state to decrement the match timer. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnInGameTimerTick();

	/*********************************************************************************************
	 * Mouse Visibility
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the current mouse visibility: is Visible when the mouse is shown. */
	void SetMouseVisibility(ESlateVisibility NewMouseVisibility) { UE_MVVM_SET_PROPERTY_VALUE(MouseVisibility, NewMouseVisibility); }
	ESlateVisibility GetMouseVisibility() const { return MouseVisibility; }

protected:
	/** Is Visible when the mouse is shown. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "[Bomber]")
	ESlateVisibility MouseVisibility = ESlateVisibility::Hidden;

	/** Called when mouse became shown or hidden. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnMouseVisibilityChanged(bool bIsShown);

	/*********************************************************************************************
	 * Can Restart Game
	 ********************************************************************************************* */
public:
	/** Setter and Getter about does running game allow to be restarted. */
	void SetCanRestartGame(const bool bNewIsPartyLeader) { UE_MVVM_SET_PROPERTY_VALUE(bCanRestart, bNewIsPartyLeader); }
	bool GetCanRestartGame() const { return bCanRestart; }

	/** Returns true if the match can be started or restarted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanStartGame() const;

	/** Is interval in seconds between ticks of both Starting (3-2-1-GO) and In-Game (120...0) timers. */
	static constexpr float DefaultTimerIntervalSec = 1.f;

protected:
	/** Determines if the player is the party leader */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter = "SetCanRestartGame", Getter = "GetCanRestartGame", Category = "[Bomber]")
	bool bCanRestart = true;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	 * Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnLocalPawnReady(const struct FGameplayEventData& Payload);
};