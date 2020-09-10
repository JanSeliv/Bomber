// Copyright 2020 Yevhenii Selivanov

#pragma once

#include "GameFramework/GameState.h"
#include "Bomber.h"
//---
#include "MyGameStateBase.generated.h"

/**
 *
 */
UCLASS()
class BOMBER_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);
	/** Called when the current game state was changed. */
	UPROPERTY(BlueprintAssignable, Category = "C++")
	FOnGameStateChanged OnGameStateChanged;

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyGameStateBase();

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Returns the AMyGameStateBase::CurrentGameState property. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE ECurrentGameState GetGameState() const { return CurrentGameStateInternal; }

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetStartingCountdown() const { return StartingCountdownInternal; }

	/** Returns the AMyGameStateBase::InGameCountdown property. Used by BPW_InGameWidget. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetInGameCountdown() const { return InGameCountdownInternal; };

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Store the game state for the current game. */
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = "OnRep_CurrentGameState", meta = (BlueprintProtected, DisplayName = "Current Game State"))
	ECurrentGameState CurrentGameStateInternal;

	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting Countdown"))
	float StartingCountdownInternal = 3.F;

	/** Seconds to the end of the round, each second decrements by 1. Replicated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, meta = (BlueprintProtected,  DisplayName = "In-Game Countdown"))
	float InGameCountdownInternal = 120.F; //[B]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on the AMyGameStateBase::CurrentGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

	/** */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStarting();

	/** Decrement the countdown timer of the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "C++", meta = (DisplayName = "Start In-Game Countdown"))
	void ServerStartInGameCountdown();
};
