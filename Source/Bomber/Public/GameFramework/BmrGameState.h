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

	/*********************************************************************************************
	 * Game State Tree
	 * Can be tracked both on host and client by binding with BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
	 ********************************************************************************************* */
public:
	/** Returns the Game State that is currently applied. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrCurrentGameState GetCurrentGameState();

	/** Returns the Game State that was applied before the current one.
	 * Is useful to check from which state the game was transitioned
	 * E.g: if current is GameStarting, but previous is InGame, but not Menu, then it means the game was restarted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	static EBmrCurrentGameState GetPreviousGameState();

	/** Returns true if the game state State Tree can be started, is false when in Render Movie cinematic mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool CanStartGameStateTree() const;

	/** Initializes the State Tree, that is used to manage the overall game state.
	 * Can be running only on the server and replicates as the tags. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void StartGameStateTree();

	/** Stops the State Tree that manages the overall game state. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void StopGameStateTree();

protected:
	/** Hosts the game state State Tree asset, running only on server and replicates as the tags. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UStateTreeComponent> GameStateTreeComponent = nullptr;

	/** Is the replicated game state, set only on the server via SetGameState, replicated to clients via OnRep. */
	UPROPERTY(Transient, ReplicatedUsing = "OnRep_CurrentGameState")
	EBmrCurrentGameState ReplicatedGameState = EBmrCurrentGameState::None;

	/** Is not-replicated local game state that always stores the previous one to track from which state the game was transitioned.
	 * Is updated at the end of ApplyGameState, so during the broadcast listeners see the correct previous state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, AdvancedDisplay, meta = (BlueprintProtected))
	EBmrCurrentGameState LocalPreviousGameState = EBmrCurrentGameState::None;

	/** Is the only proper way to change the game state, called by the State Tree on the server.
	 * No one should change the game state directly, all transitions are managed by the State Tree.
	 * Can be also changed by `Bomber.Game.SetGameState VALUE` cheat command. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetGameState(EBmrCurrentGameState NewGameState);

	/** Updates current game state. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyGameState();

	/** Called on the ABmrGameState::ReplicatedGameState property updating. */
	UFUNCTION()
	void OnRep_CurrentGameState();

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
	void OnLocalPawnReady(const struct FGameplayEventData& Payload);
};