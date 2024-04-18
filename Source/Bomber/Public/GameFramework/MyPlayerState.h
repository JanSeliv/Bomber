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
UCLASS(Config = "GameUserSettings", DefaultConfig)
class BOMBER_API AMyPlayerState final : public APlayerState
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	AMyPlayerState();

	/** Returns true if this Player State is controlled by a locally controlled player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	bool IsPlayerStateLocallyControlled() const;

	/*********************************************************************************************
	 * End Game State
	 * Is personal for each player: Win, Lose or Draw.
	 * Can be tracked by listening UGlobalEventsSubsystem::Get().OnEndGameStateChanged
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGameStateChanged, EEndGameState, EndGameState);

	/** Called when player's match result was changed (Win, lose, draw or none applied). */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnEndGameStateChanged OnEndGameStateChanged;

	/** Returns result of the game for controlled player after ending the game. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE EEndGameState GetEndGameState() const { return EndGameStateInternal; }

	/** Updates result of the game for controlled player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void UpdateEndGameState();

protected:
	/** Contains result of the game for controlled player after ending the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "End Game State"))
	EEndGameState EndGameStateInternal = EEndGameState::None;

	/** Set new End-Game state, is made as multicast to notify own client asap. */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "C++", meta = (BlueprintProtected))
	void MulticastSetEndGameState(EEndGameState NewEndGameState);

	/*********************************************************************************************
	 * Player Name
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, FName, NewName);

	/** Called when player name is changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPlayerNameChanged OnPlayerNameChanged;

	/** Sets locally the player name on each nickname change.*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerNameCustom(FName NewName);

	/** Returns custom player name. */
	UFUNCTION(BlueprintPure, Category = "C++", meta = (DisplayName = "Get Player Name Custom"))
	FName GetPlayerFNameCustom() const;

	/** Is overriden to return own player name that is saved to config. */
	virtual FORCEINLINE FString GetPlayerNameCustom() const override { return GetPlayerFNameCustom().ToString(); }

protected:
	/** Replaces APlayerState::PlayerNamePrivate for saving purposes, since original property is not 'Config'.
	 * Can contain different languages, uppercase, lowercase etc, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, ReplicatedUsing = "OnRep_CustomPlayerName", Category = "C++", meta = (BlueprintProtected, DisplayName = "Custom Player Name"))
	FName CustomPlayerNameInternal;

	/** Called on server when settings are saved to apply local player name. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (BlueprintProtected))
	void ServerSetPlayerNameCustom(FName NewName);

	/** Applies and broadcasts player name. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCustomPlayerName();

	/** Called on client when custom player name is changed. */
	UFUNCTION()
	void OnRep_CustomPlayerName();

	/*********************************************************************************************
	 * Is Character Dead
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDeadChanged, bool, bIsCharacterDead);

	/** Called when character dead status is changed: character was killed or revived. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnCharacterDeadChanged OnCharacterDeadChanged;

	/** Returns true if current player is alive.*/
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsCharacterDead() const { return bIsCharacterDeadInternal; }

	/** Sets character dead status, true if was killed, false if was revived. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCharacterDead(bool bIsDead);

protected:
	/** Is true when player is dead. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_IsCharacterDead", Category = "C++", meta = (BlueprintProtected, DisplayName = "Is Character Dead"))
	bool bIsCharacterDeadInternal = false;

	/** Called on client when character Dead status is changed. */
	UFUNCTION()
	void OnRep_IsCharacterDead();

	/** Applies and broadcasts Is Character Dead status. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyIsCharacterDead();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Listens game states to notify server about ending game for controlled player. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Listens game settings to apply them once saved. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnSaveSettings();
};