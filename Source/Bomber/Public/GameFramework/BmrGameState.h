// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/GameState.h"

// UE
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"

#include "BmrGameState.generated.h"

/**
 * Own implementation of managing the game's global state.
 * @see Access its data with UGameStateDataAsset (Content/Bomber/DataAssets/DA_GameState).
 */
UCLASS()
class BOMBER_API ABmrGameState final : public AGameStateBase,
                                       public IAbilitySystemInterface,
                                       public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	ABmrGameState();

	/** Returns the current game state, it will crash if can't be obtained, should be used only when the game is running. */
	static ABmrGameState& Get();

	/*********************************************************************************************
	 * Game State Tree
	 * SET:
	 * - Update ST_GameState asset with new state and transition rules.
	 * GET:
	 * - in code, use ABmrGameState::Get().HasMatchingGameplayTag(FBmrGameStateTag::{Tag}).
	 * - In blueprints, use 'Get BMR Game State' -> 'Has Matching Gameplay Tag' nodes.
	 * LISTEN:
	 * - in code, call BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
	 * - in blueprints, call 'Listen Gameplay Message' node to 'Event.GameState.Changed' tag.
	 ********************************************************************************************* */
public:
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

	/** Registers to listen for GameState tag changes on ASC, on both server and client. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void BindOnGameStateTagChanged();

	/** Broadcasts the BmrGameplayTags::Event::GameState_Changed event via Gameplay Message Router.
	 * Is called automatically when GameState tag changes on ASC (both server and client).
	 * Payload.InstigatorTags contains the current GameState tag, so listeners can check via Payload.InstigatorTags.HasTag(). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void BroadcastGameStateChanged();

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
public:
	/** Returns the Ability System Component from the Generated Map.
	 * In blueprints, call 'Get Ability System Component' as interface function. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAbilitySystemComponent& GetAbilitySystemComponentChecked() const;

	/** Returns the gameplay tags owned by this actor via IGameplayTagAssetInterface. */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

protected:
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