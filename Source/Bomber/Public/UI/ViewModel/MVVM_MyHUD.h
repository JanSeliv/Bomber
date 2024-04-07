// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/ViewModel/MVVM_MyBaseViewModel.h"
//---
#include "MVVM_MyHUD.generated.h"

/**
 * Contains UI data to be used only by the HUD widget.
 */
UCLASS(DisplayName = "My HUD View Model")
class BOMBER_API UMVVM_MyHUD : public UMVVM_MyBaseViewModel
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Countdown timers
	 ********************************************************************************************* */
public:
	/** Setter and Getter about the summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	void SetStartingTimerSecRemain(const FText& NewStartingTimerSecRemain);
	const FText& GetStartingTimerSecRemain() const { return StartingTimerSecRemain; }

	/** Setter and Getter about the seconds to the end of the round. */
	void SetInGameTimerSecRemain(const FText& NewInGameTimerSecRemain);
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
	 * PowerUps
	 ********************************************************************************************* */
public:
	/** Setter and Getter about current amount of speed power-ups. */
	void SetPowerUpSkate(const FText& NewPowerUpSkate);
	const FText& GetPowerUpSkate() const { return PowerUpSkate; }

	/** Setter and Getter about current amount of max bomb power-ups. */
	void SetPowerUpBomb(const FText& NewPowerUpBomb);
	const FText& GetPowerUpBomb() const { return PowerUpBomb; }

	/** Setter and Getter about current amount of blast radius power-ups. */
	void SetPowerUpFire(const FText& NewPowerUpFire);
	const FText& GetPowerUpFire() const { return PowerUpFire; }

protected:
	/** Current amount of speed power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpSkate = FText::GetEmpty();

	/** Current amount of max bomb power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpBomb = FText::GetEmpty();

	/** Current amount of blast radius power-ups. */
	UPROPERTY(BlueprintReadWrite, Transient, FieldNotify, Setter, Getter, Category = "C++")
	FText PowerUpFire = FText::GetEmpty();

	/** Called when power-ups were updated. */
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

	/** Called when local player character was possessed, so we can bind to data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalPlayerReady(class APlayerCharacter* PlayerCharacter);
};