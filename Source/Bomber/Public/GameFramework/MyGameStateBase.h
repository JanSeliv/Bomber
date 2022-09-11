// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameState.h"
//---
#include "Bomber.h"
//---
#include "MyGameStateBase.generated.h"

/**
 * The data of the game match.
 */
UCLASS()
class BOMBER_API UGameStateDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Game State data asset. */
	static const UGameStateDataAsset& Get();

	/** Returns general value how ofter update actors and states in the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetTickInterval() const { return TickInternal; }

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetStartingCountdown() const { return StartingCountdownInternal; }

	/** Returns the left second to the end of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetInGameCountdown() const { return InGameCountdownInternal; }

protected:
	/** General value how ofter update actors and states in the game. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Tick Interval", ShowOnlyInnerProperties))
	float TickInternal = 0.2F; //[D]

	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "Starting Countdown"))
	int32 StartingCountdownInternal = 3; //[D]

	/** Seconds to the end of the round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (BlueprintProtected, DisplayName = "In-Game Countdown"))
	int32 InGameCountdownInternal = 120; //[D]
};

/**
 * Own implementation of managing the game's global state.
 */
UCLASS()
class BOMBER_API AMyGameStateBase final : public AGameStateBase
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public properties
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, ECurrentGameState, CurrentGameState);

	/** Called when the current game state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnGameStateChanged OnGameStateChanged; //[DMD]

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyGameStateBase();

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++")
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Returns the AMyGameStateBase::CurrentGameState property. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ECurrentGameState GetCurrentGameState();

	/** Returns the left second of the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetStartingTimerSecondsRemain() const { return StartingTimerSecRemainInternal; }

	/** Returns the left second to the end of the match. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE float GetInGameTimerSecondsRemain() const { return InGameTimerSecRemainInternal; }

	/** Returns true if 'Three-two-one-GO' timer was already finished, so the match was started. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsStartingTimerElapsed() const { return FMath::IsNearlyZero(StartingTimerSecRemainInternal) || FMath::IsNegative(StartingTimerSecRemainInternal); }

	/** Returns true if there are no seconds remain to the end of the match, so the match was ended. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool IsInGameTimerElapsed() const { return FMath::IsNearlyZero(InGameTimerSecRemainInternal) || FMath::IsNegative(InGameTimerSecRemainInternal); }

	/** Returns true if there request to update the End-Game state for players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE bool DoesWantUpdateEndState() const { return bWantsUpdateEndStateInternal; }

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Store the game state for the current game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CurrentGameState", meta = (BlueprintProtected, DisplayName = "Current Game State"))
	ECurrentGameState CurrentGameStateInternal = ECurrentGameState::None; //[G]

	/** Handles time counting in the game.*/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Countdown Timer"))
	FTimerHandle CountdownTimerInternal; //[G]

	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Replicated, meta = (BlueprintProtected, DisplayName = "Starting Timer Seconds Remain"))
	float StartingTimerSecRemainInternal = 0.F; //[G]

	/** Seconds to the end of the round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Replicated, meta = (BlueprintProtected, DisplayName = "In-Game Timer Seconds Remain"))
	float InGameTimerSecRemainInternal = 0.F; //[G]

	/** Is true where there request to update the End-Game state for players */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Wants Update End State"))
	bool bWantsUpdateEndStateInternal = false; //[G]

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the AMyGameStateBase::CurrentGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

	/** Called to starting counting different time in the game. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void TriggerCountdowns();

	/** Is called each UGameStateDataAsset::TickInternal to count different time in the game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCountdownTimerTicked();

	/** Is called during the Game Starting state to handle the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DecrementStartingCountdown();

	/** Is called during the In-Game state to handle time consuming for the current match. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DecrementInGameCountdown();

	/** Is called during the In-Game state to try to register the End-Game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateEndGameStates();

	/** Called when any player or bot was exploded. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAnyCharacterDestroyed();
};
