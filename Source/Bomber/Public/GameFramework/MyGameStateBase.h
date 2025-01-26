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

	/** Is interval in seconds between ticks of both Starting (3-2-1-GO) and In-Game (120...0) timers. */
	static constexpr float DefaultTimerIntervalSec = 1.f;

	/*********************************************************************************************
	 * Game State
	 * Can be tracked both on host and client by binding with BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged); 
	 ********************************************************************************************* */
public:
	/** Returns true if current game state can be eventually changed. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool CanChangeGameState(ECurrentGameState NewGameState) const;

	/** Set the new game state for the current game.
	 * Should be called only on the server.
	 * Clients can change the game state only from AMyPlayerController.
	 * Can be also changed by `Bomber.Game.SetGameState VALUE` cheat command. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetGameState(ECurrentGameState NewGameState);

	/** Returns the Game State that is currently applied. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ECurrentGameState GetCurrentGameState();

	/** Returns the Game State that was applied before the current one.
	 * Is useful to check from which state the game was transitioned
	 * E.g: if current is GameStarting, but previous is InGame, but not Menu, then it means the game was restarted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ECurrentGameState GetPreviousGameState();

protected:
	/** Is read-only local version of the game state that is not replicated, can be read on both server and client, but never should be set directly.
	 * Is populated in order to allow local clients apply (update) the game state before it will be replicated.
	 * @warning Do not set it directly, use AMyGameStateBase::ServerSetGameState() instead to set ReplicatedGameStateInternal. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "Current Game State"))
	ECurrentGameState LocalGameStateInternal = ECurrentGameState::None;

	/** Is write-only replicated version of the game state, can be set only on the server but never should be read.
	 * @warning Do not read it directly, use AMyGameStateBase::GetCurrentGameState() instead to read LocalGameStateInternal. */
	UPROPERTY(Transient, ReplicatedUsing = "OnRep_CurrentGameState")
	ECurrentGameState ReplicatedGameStateInternal = ECurrentGameState::None;

	/** Is not-replicated local game state that always stores the previous one to track from which state the game was transitioned. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "Previous Game State"))
	ECurrentGameState LocalPreviousGameStateInternal = ECurrentGameState::None;

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the AMyGameStateBase::CurrentGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

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

	/** Starts counting the 3-2-1-GO timer when match is starting, can be called both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TriggerStartingCountdown();

	/** Clears the Starting timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopStartingCountdown();

protected:
	/** Remaining seconds of launching 'Three-two-one-GO' timer that is used on game starting.
	 * Is not replicated, since is triggered locally for everyone. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "Starting Timer Seconds Remain"))
	float StartingTimerSecRemainInternal = 0.F;

	/** Handles time counting during the Game Starting state. */
	FTimerHandle StartingTimerInternal;

	/** Is called once a second during the Game Starting state to decrement the 'Three-two-one-GO' timer, both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnStartingTimerTick();

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

	/** Starts counting the (120...0) timer during the match, can be called both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void TriggerInGameCountdown();

	/** Clears the In-Game timer and stops counting it. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void StopInGameCountdown();

protected:
	/** Seconds to the end of the round.
	 * Is not replicated, since is triggered locally for everyone. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, meta = (BlueprintProtected, DisplayName = "In-Game Timer Seconds Remain"))
	float InGameTimerSecRemainInternal = 0.F;

	/** Handles time counting during the In-Game state. */
	FTimerHandle InGameTimerInternal;

	/** Is called once a second during the In-Game state to decrement the match timer, both on the server and clients. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnInGameTimerTick();

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

	/** Called when the local player character is spawned, possessed, and replicated. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnLocalCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);
};