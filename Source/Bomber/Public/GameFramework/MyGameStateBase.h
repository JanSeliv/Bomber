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
class UGameStateDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the Game State data asset. */
	static const UGameStateDataAsset& Get();

	/** Return the summary time required to start the 'Three-two-one-GO' timer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetStartingCountdown() const { return StartingCountdownInternal; }

	/** Returns the left second to the end of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetInGameCountdown() const { return InGameCountdownInternal; }

protected:
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
class AMyGameStateBase final : public AGameStateBase
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnyPlayerDestroyed);

	/** Called when one of players was destroyed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnAnyPlayerDestroyed OnAnyPlayerDestroyed; //[DMD]

	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Default constructor. */
	AMyGameStateBase();

	/** Set the new game state for the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (DisplayName = "Set Game State"))
	void ServerSetGameState(ECurrentGameState NewGameState);

	/** Returns the AMyGameStateBase::CurrentGameState property. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	static ECurrentGameState GetCurrentGameState();

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Store the game state for the current game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CurrentGameState", meta = (BlueprintProtected, DisplayName = "Current Game State"))
	ECurrentGameState CurrentGameStateInternal = ECurrentGameState::None;

	/** Decrement AMyGameStateBase::StartingCountdownInternal by 1 for each second.*/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting Timer"))
	FTimerHandle StartingTimerInternal;

	/** Decrement AMyGameStateBase::InGameCountdownInternal by 1 for each second. */
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BlueprintProtected, DisplayName = "In-Game Timer"))
	FTimerHandle InGameTimerInternal;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the AMyGameStateBase::CurrentGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

	/** Called when game enters to the Game Starting state and timer starts countdown. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (BlueprintProtected))
	void ServerOnGameStarting();

	/** Decrement the countdown timer of the current game. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (DisplayName = "Start In-Game Countdown"))
	void ServerStartInGameCountdown();
};
