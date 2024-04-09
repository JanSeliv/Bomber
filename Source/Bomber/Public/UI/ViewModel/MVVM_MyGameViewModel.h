// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Bomber.h"
#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "MVVM_MyGameViewModel.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Contains general data to be used only by widgets. 
 */
UCLASS(DisplayName = "My Game View Model")
class BOMBER_API UMVVM_MyGameViewModel : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

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
	 * Events
	 ********************************************************************************************* */
protected:
	/**  Is called when this View Model is constructed.
	* Is used for bindings to the changes in other systems in order to update own data. */
	virtual void OnViewModelConstruct_Implementation(const UUserWidget* UserWidget) override;

	/** Is called when this View Model is destructed. */
	virtual void OnViewModelDestruct_Implementation() override;

	/** Called when local player character was possessed, so we can bind to data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterWithIDPossessed(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};