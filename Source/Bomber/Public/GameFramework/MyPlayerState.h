// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/PlayerState.h"
//---
#include "Bomber.h"
//---
#include "MyPlayerState.generated.h"

/**
 * The player state of a bomber player.
 */
UCLASS(Config = "GameUserSettings")
class BOMBER_API AMyPlayerState final : public APlayerState
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGameStateChanged, EEndGameState, EndGameState);

	/** Called when the player state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnEndGameStateChanged OnEndGameStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, FName, NewName);

	/** Called when player name is changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnPlayerNameChanged OnPlayerNameChanged;

	/** Default constructor. */
	AMyPlayerState();

	/** Returns result of the game for controlled player after ending the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE EEndGameState GetEndGameState() const { return EndGameStateInternal; }

	/** Is created to expose APlayerState::SetPlayerName(NewName) to blueprints. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerNameCustom(FName NewName);

	/** Returns custom player name. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get Player Name Custom"))
	FName GetPlayerFNameCustom() const;

	/** Is overriden to return own player name that is saved to config. */
	virtual FString GetPlayerNameCustom() const override { return GetPlayerFNameCustom().ToString(); }

	/** Updates result of the game for controlled player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void UpdateEndGameState();

	/** Returns true if current player is alive.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsPlayerAlive() const;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Contains result of the game for controlled player after ending the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "End Game State"))
	EEndGameState EndGameStateInternal = EEndGameState::None;

	/** Config: custom name set by player.
	 * Can contain different languages, uppercase, lowercase etc, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Custom Player Name"))
	FName CustomPlayerNameInternal;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Listen game states to notify server about ending game for controlled player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Set new End-Game state, is made as multicast to notify own client asap. */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected))
	void MulticastSetEndGameState(EEndGameState NewEndGameState);
};
