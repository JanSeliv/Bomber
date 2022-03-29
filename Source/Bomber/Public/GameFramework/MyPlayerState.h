// Copyright 2021 Yevhenii Selivanov

#pragma once

#include "GameFramework/PlayerState.h"
//---
#include "Bomber.h"
#include "Components/MySkeletalMeshComponent.h"
//---
#include "MyPlayerState.generated.h"

/**
 * The player state of a bomber player.
 */
UCLASS(Config = "GameUserSettings")
class AMyPlayerState final : public APlayerState
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public
	* --------------------------------------------------- */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGameStateChanged, EEndGameState, EndGameState);

	/** Called when the player state was changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnEndGameStateChanged OnEndGameStateChanged; //[DMD]

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, FName, NewName);

	/** Called when player name is changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "C++")
	FOnPlayerNameChanged OnPlayerNameChanged; //[DMD]

	/** Default constructor. */
	AMyPlayerState();

	/** Returns result of the game for controlled player after ending the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE EEndGameState GetEndGameState() const { return EndGameStateInternal; }

	/** Set and apply how a player has to look like.
	 * @param CustomPlayerMeshData New data to apply. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void ServerSetCustomPlayerMeshData(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/** By which level type a skeletal mesh is chosen. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

	/** Is created to expose APlayerState::SetPlayerName(NewName) to blueprints. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetPlayerNameCustom(FName NewName);

	/** Returns custom player name. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++", meta = (DisplayName = "Get Player Name Custom"))
	FName GetPlayerFNameCustom() const;

	/** Is overriden to return own player name that is saved to config. */
	virtual FString GetPlayerNameCustom() const override { return GetPlayerFNameCustom().ToString(); }

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	/** Contains result of the game for controlled player after ending the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "End Game State"))
	EEndGameState EndGameStateInternal = EEndGameState::None; //[G]

	/** A level type of chosen skeletal mesh. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_PlayerMeshData", Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal; //[G]

	/** Config: custom name set by player.
	 * Can contain different languages, uppercase, lowercase etc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Replicated, Category = "C++", meta = (BlueprintProtected, DisplayName = "Custom Player Name"))
	FName CustomPlayerNameInternal; //[С]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when the game starts. Created widget. */
	virtual void BeginPlay() override;

	/** Listen game states to notify server about ending game for controlled player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Updated result of the game for controlled player after ending the game. Called when one of players is destroying. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (BlueprintProtected))
	void ServerUpdateEndState();

	/** Updates current player mesh data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyPlayerMeshData();

	/** Respond on changes in player mesh data to reset to set the mesh on client. */
	UFUNCTION()
	void OnRep_PlayerMeshData();
};
