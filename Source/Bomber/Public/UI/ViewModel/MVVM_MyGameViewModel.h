// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "Bomber.h"
//---
#include "Components/SlateWrapperTypes.h"
//---
#include "MVVM_MyGameViewModel.generated.h"

/**
 * Contains general data to be used only by widgets. 
 */
UCLASS(DisplayName = "My Game View Model")
class BOMBER_API UMVVM_MyGameViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Game State
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
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
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
	 * Local Player's Power-Ups
	 ********************************************************************************************* */
public:
	/** Setter and Getter about current amount of speed power-ups. */
	void SetPowerUpSkateN(const FText& NewPowerUpSkate) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpSkateN, NewPowerUpSkate); }
	const FText& GetPowerUpSkateN() const { return PowerUpSkateN; }

	/** Setter and Getter about current amount of max bomb power-ups. */
	void SetPowerUpBombN(const FText& NewPowerUpBomb) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpBombN, NewPowerUpBomb); }
	const FText& GetPowerUpBombN() const { return PowerUpBombN; }

	/** Setter and Getter about current amount of blast radius power-ups. */
	void SetPowerUpFireN(const FText& NewPowerUpFire) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpFireN, NewPowerUpFire); }
	const FText& GetPowerUpFireN() const { return PowerUpFireN; }

	/** Setter and Getter about percentage of the current amount of speed power-ups. */
	void SetPowerUpSkatePercent(float NewPowerUpSkatePercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpSkatePercent, NewPowerUpSkatePercent); }
	float GetPowerUpSkatePercent() const { return PowerUpSkatePercent; }

	/** Setter and Getter about percentage of the current amount of max bomb power-ups. */
	void SetPowerUpBombPercent(float NewPowerUpBombPercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpBombPercent, NewPowerUpBombPercent); }
	float GetPowerUpBombPercent() const { return PowerUpBombPercent; }

	/** Setter and Getter about percentage of the current amount of blast radius power-ups. */
	void SetPowerUpFirePercent(float NewPowerUpFirePercent) { UE_MVVM_SET_PROPERTY_VALUE(PowerUpFirePercent, NewPowerUpFirePercent); }
	float GetPowerUpFirePercent() const { return PowerUpFirePercent; }

protected:
	/** Current amount of speed power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpSkateN = FText::GetEmpty();

	/** Current amount of max bomb power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpBombN = FText::GetEmpty();

	/** Current amount of blast radius power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpFireN = FText::GetEmpty();

	/** Percentage of the current speed power-up. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpSkatePercent = 0.f;

	/** Percentage of the current amount of max bomb power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpBombPercent = 0.f;

	/** Percentage of the current amount of blast radius power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	float PowerUpFirePercent = 0.f;

	/** Called when power-ups were updated on local character. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPowerUpsChanged(const struct FPowerUp& NewPowerUps);

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

	/** Called when local player character was possessed, so we can bind to data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};