// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameState.h"
//---
#include "Bomber.h"
//---
#include "MyGameStateBase.generated.h"

/**
 * Own implementation of managing the game's global state.
 * @see Access its data with UGameStateDataAsset (Content/Bomber/DataAssets/DA_GameState).
 */
UCLASS()
class BOMBER_API AMyGameStateBase final : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	AMyGameStateBase();

	/** Returns the current game state, it will crash if can't be obtained, should be used only when the game is running. */
	static AMyGameStateBase& Get();

	/*********************************************************************************************
	 * Current Game State enum
	 * Can be tracked both on host and client by binding with BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); 
	 ********************************************************************************************* */
public:
	/** Set the new game state for the current game.*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++")
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Returns the AMyGameStateBase::CurrentGameState property. */
	UFUNCTION(BlueprintPure, Category = "C++")
	static ECurrentGameState GetCurrentGameState();

protected:
	/** Store the game state for the current game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CurrentGameState", AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "Current Game State"))
	ECurrentGameState CurrentGameStateInternal = ECurrentGameState::None;

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the AMyGameStateBase::CurrentGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

	/*********************************************************************************************
	 * End-Game State enum
	 * Result of finished match (Win, Lose or Draw)
	 ********************************************************************************************* */
public:
	/** Returns true if there request to update the End-Game state for players. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool DoesWantUpdateEndState() const { return bWantsUpdateEndStateInternal; }

protected:
	/** Is true where there request to update the End-Game state for players */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Wants Update End State"))
	bool bWantsUpdateEndStateInternal = false;

	/** Is called during the In-Game state to try to register the End-Game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateEndGameStates();

	/** Called when any player or bot was exploded. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAnyCharacterDestroyed();

	/*********************************************************************************************
	 * Starting Timer
	 * 3-2-1-GO
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartingTimerSecRemainChanged, float, NewStartingTimerSecRemain);

	/** Called when the 'Three-two-one-GO' timer was updated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnStartingTimerSecRemainChanged OnStartingTimerSecRemainChanged;

	/** Returns the left second of the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetStartingTimerSecondsRemain() const { return StartingTimerSecRemainInternal; }

	/** Returns true if 'Three-two-one-GO' timer was already finished, so the match was started. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsStartingTimerElapsed() const { return FMath::IsNearlyZero(StartingTimerSecRemainInternal) || StartingTimerSecRemainInternal < 0.f; }

	/** Sets the left second of the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetStartingTimerSecondsRemain(float NewStartingTimerSecRemain);

protected:
	/** The summary seconds of launching 'Three-two-one-GO' timer that is used on game starting. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_StartingTimerSecRemain", AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "Starting Timer Seconds Remain"))
	float StartingTimerSecRemainInternal = 0.F;

	/** Is called on client when the 'Three-two-one-GO' timer was updated. */
	UFUNCTION()
	void OnRep_StartingTimerSecRemain();

	/** Updates current starting timer seconds remain. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyStartingTimerSecondsRemain();

	/** Is called during the Game Starting state to handle the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DecrementStartingCountdown();

	/*********************************************************************************************
	 * In-Game Timer
	 * Runs during the match (120...0)
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInGameTimerSecRemainChanged, float, NewInGameTimerSecRemain);

	/** Called when remain seconds to the end of the match timer was updated. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnInGameTimerSecRemainChanged OnInGameTimerSecRemainChanged;

	/** Returns the left second to the end of the match. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE float GetInGameTimerSecondsRemain() const { return InGameTimerSecRemainInternal; }

	/** Returns true if there are no seconds remain to the end of the match, so the match was ended. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsInGameTimerElapsed() const { return FMath::IsNearlyZero(InGameTimerSecRemainInternal) || InGameTimerSecRemainInternal < 0.f; }

	/** Sets the left second to the end of the match. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetInGameTimerSecondsRemain(float NewInGameTimerSecRemain);

protected:
	/** Seconds to the end of the round. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_InGameTimerSecRemain", AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "In-Game Timer Seconds Remain"))
	float InGameTimerSecRemainInternal = 0.F;

	/** Is called on client when in-match timer was updated. */
	UFUNCTION()
	void OnRep_InGameTimerSecRemain();

	/** Updates current in-match timer seconds remain. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyInGameTimerSecondsRemain();

	/** Is called during the In-Game state to handle time consuming for the current match. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void DecrementInGameCountdown();

	/*********************************************************************************************
	 * Countdown
	 * Is used by both 'Three-two-one-GO' and In-Game timers
	 ********************************************************************************************* */
protected:
	/** Handles time counting in the game.*/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Countdown Timer"))
	FTimerHandle CountdownTimerInternal;

	/** Called to starting counting different time in the game. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void TriggerCountdowns();

	/** Is called each UGameStateDataAsset::TickInternal to count different time in the game. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCountdownTimerTicked();

	/*********************************************************************************************
	 * Multiplayer helpers
	 ********************************************************************************************* */
public:
	/** Returns true if any client is connected to the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsMultiplayerGame() const { return GetPlayersInMultiplayerNum() > 1; }

	/** Returns amount of players (host + clients) playing this game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	int32 GetPlayersInMultiplayerNum() const;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** This is called only in the gameplay before calling begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Overridable function called whenever this actor is being removed from a level. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Enables or disable all game features.
	 * @see UGameStateDataAsset::GetGameFeaturesToEnable() */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetGameFeaturesEnabled(bool bEnable);
};