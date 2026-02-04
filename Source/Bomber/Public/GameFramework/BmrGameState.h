// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameState.h"

// Bomber
#include "Bomber.h" // EBmrCurrentGameState

#include "BmrGameState.generated.h"

/**
 * Own implementation of managing the game's global state.
 * @see Access its data with UGameStateDataAsset (Content/Bomber/DataAssets/DA_GameState).
 */
UCLASS()
class BOMBER_API ABmrGameState final : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	ABmrGameState();

	/** Returns the current game state, it will crash if can't be obtained, should be used only when the game is running. */
	static ABmrGameState& Get();

	/** Is interval in seconds between ticks of both Starting (3-2-1-GO) and In-Game (120...0) timers. */
	static constexpr float DefaultTimerIntervalSec = 1.f;

	/*********************************************************************************************
	 * Game State
	 * Can be tracked both on host and client by binding with BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
	 ********************************************************************************************* */
public:
	/** Returns true if current game state can be eventually changed. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanChangeGameState(EBmrCurrentGameState NewGameState) const;

	/** Set the new game state for the current game.
	 * Should be called only on the server.
	 * Clients can change the game state only from ABmrPlayerController.
	 * Can be also changed by `Bomber.Game.SetGameState VALUE` cheat command. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetGameState(EBmrCurrentGameState NewGameState);

	/** Returns the Game State that is currently applied. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrCurrentGameState GetCurrentGameState();

	/** Returns the Game State that was applied before the current one.
	 * Is useful to check from which state the game was transitioned
	 * E.g: if current is GameStarting, but previous is InGame, but not Menu, then it means the game was restarted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrCurrentGameState GetPreviousGameState();

	/** Returns true if the match can be started or restarted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanStartGame() const;

protected:
	/** Is read-only local version of the game state that is not replicated, can be read on both server and client, but never should be set directly.
	 * Is populated in order to allow local clients apply (update) the game state before it will be replicated.
	 * @warning Do not set it directly, use ABmrGameState::ServerSetGameState() instead to set ReplicatedGameState. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	EBmrCurrentGameState LocalGameState = EBmrCurrentGameState::None;

	/** Is write-only replicated version of the game state, can be set only on the server but never should be read.
	 * @warning Do not read it directly, use ABmrGameState::GetCurrentGameState() instead to read LocalGameState. */
	UPROPERTY(Transient, ReplicatedUsing = "OnRep_CurrentGameState")
	EBmrCurrentGameState ReplicatedGameState = EBmrCurrentGameState::None;

	/** Is not-replicated local game state that always stores the previous one to track from which state the game was transitioned. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	EBmrCurrentGameState LocalPreviousGameState = EBmrCurrentGameState::None;

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the ABmrGameState::ReplicatedGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

	/*********************************************************************************************
	 * Starting Timer
	 * 3-2-1-GO
	 ********************************************************************************************* */
public:
	/** Returns true if 'Three-two-one-GO' timer was already finished, so the match was started. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsStartingTimerElapsed() const { return FMath::IsNearlyZero(StartingTimerSecRemain) || StartingTimerSecRemain < 0.f; }

	/** Sets the left second of the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetStartingTimerSecondsRemain(float NewStartingTimerSecRemain);

	/** Starts counting the 3-2-1-GO timer when match is starting, can be called both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TriggerStartingCountdown();

	/** Clears the Starting timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void StopStartingCountdown();

protected:
	/** Remaining seconds of launching 'Three-two-one-GO' timer that is used on game starting.
	 * Is not replicated, since is triggered locally for everyone. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	float StartingTimerSecRemain = 0.f;

	/** Handles time counting during the Game Starting state. */
	FTimerHandle StartingTimer;

	/** Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer, both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnStartingTimerTick();

	/*********************************************************************************************
	 * In-Game Timer
	 * Runs during the match (120...0)
	 ********************************************************************************************* */
public:
	/** Returns true if there are no seconds remain to the end of the match, so the match was ended. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsInGameTimerElapsed() const { return FMath::IsNearlyZero(InGameTimerSecRemain) || InGameTimerSecRemain < 0.f; }

	/** Sets the left second to the end of the match. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetInGameTimerSecondsRemain(float NewInGameTimerSecRemain);

	/** Starts counting the (120...0) timer during the match, can be called both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void TriggerInGameCountdown();

	/** Clears the In-Game timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void StopInGameCountdown();

protected:
	/** Seconds to the end of the round.
	 * Is not replicated, since is triggered locally for everyone. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	float InGameTimerSecRemain = 0.F;

	/** Handles time counting during the In-Game state. */
	FTimerHandle InGameTimer;

	/** Is called once a second during the In-Game state to decrement the match timer, both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnInGameTimerTick();

	/*********************************************************************************************
	 * Game Difficulty
	 ********************************************************************************************* */
public:
	/** Returns the manager, which is responsible for the game difficulty settings and logic. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class UBmrGameDifficultyManagerComponent* GetGameDifficultyManager() const { return GameDifficultyManager; }

protected:
	/** Manages the game difficulty settings and logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrGameDifficultyManagerComponent> GameDifficultyManager = nullptr;

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

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnLocalPawnReady(class ABmrPawn* Pawn, int32 PlayerId);
};