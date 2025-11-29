// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"

// Bomber
#include "Bomber.h"

// UE
#include "Components/SlateWrapperTypes.h"

#include "MVVM_MyGameViewModel.generated.h"

/**
 * Contains general data to be used only by widgets.
 */
UCLASS(DisplayName = "My Game View Model")
class BOMBER_API UMVVM_MyGameViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Current Game State
	 ********************************************************************************************* */
public:
	/** Setter and Getter widgets about the current game state. */
	UFUNCTION()
	void SetCurrentGameState(ECurrentGameState NewCurrentGameState) { UE_MVVM_SET_PROPERTY_VALUE(CurrentGameState, NewCurrentGameState); }

	ECurrentGameState GetCurrentGameState() const { return CurrentGameState; }

protected:
	/** Represents the current game state.
	 * Is commonly used by 'UMyBlueprintFunctionLibrary::GetVisibilityByGameState' to show or hide own widget. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ECurrentGameState CurrentGameState = ECurrentGameState::None;

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState InGameState);

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
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ESlateVisibility EndGameStateVisibility = ESlateVisibility::Collapsed;

	/** The result of the game: Win, Lose, Draw. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText EndGameResult = FText::GetEmpty();

	/** Called when the player state was changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnEndGameStateChanged(EEndGameState NewEndGameState);

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
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText StartingTimerSecRemain = FText::GetEmpty();

	/** Seconds to the end of the round. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText InGameTimerSecRemain = FText::GetEmpty();

	/** Called when the 'Three-two-one-GO' timer was updated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnStartingTimerSecRemainChanged(float NewStartingTimerSecRemain);

	/** Called when remain seconds to the end of the match timer was updated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnInGameTimerSecRemainChanged(float NewInGameTimerSecRemain);

	/*********************************************************************************************
	 * Mouse Visibility
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the current mouse visibility: is Visible when the mouse is shown. */
	void SetMouseVisibility(ESlateVisibility NewMouseVisibility) { UE_MVVM_SET_PROPERTY_VALUE(MouseVisibility, NewMouseVisibility); }
	ESlateVisibility GetMouseVisibility() const { return MouseVisibility; }

protected:
	/** Is Visible when the mouse is shown. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	ESlateVisibility MouseVisibility = ESlateVisibility::Hidden;

	/** Called when mouse became shown or hidden. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnMouseVisibilityChanged(bool bIsShown);

	/*********************************************************************************************
	 * Can Restart Game
	 ********************************************************************************************* */
public:
	/** Setter and Getter about does running game allow to be restarted. */
	void SetCanRestartGame(const bool bNewIsPartyLeader) { UE_MVVM_SET_PROPERTY_VALUE(bCanRestart, bNewIsPartyLeader); }
	bool GetCanRestartGame() const { return bCanRestart; }

protected:
	/** Determines if the player is the party leader */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter = "SetCanRestartGame", Getter = "GetCanRestartGame", Category = "C++")
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

	/** Called when Game State was created in current world. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateCreated(class AGameStateBase* GameState);

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};