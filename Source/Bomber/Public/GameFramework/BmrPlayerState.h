// Copyright (c) Yevhenii Selivanov

#pragma once

#include "GameFramework/PlayerState.h"

// Bomber
#include "Bomber.h" // EBmrEndGameState, EBmrPlayerType
#include "Structures/BmrMeshData.h"

// UE
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"

#include "BmrPlayerState.generated.h"

enum class EBmrPlayerType : uint8;

/**
 * Holds Player's data like nickname.
 * It's replicated to all clients and persists between matches.
 * Unlike APlayerState, this class is not respawned on player join and not destroyed on player leave, but is reused for both human and bot characters.
 */
UCLASS(Config = "GameUserSettings", DefaultConfig)
class BOMBER_API ABmrPlayerState : public APlayerState,
                                   public IAbilitySystemInterface,
                                   public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	ABmrPlayerState();

	/** Returns true if this Player State is controlled by a locally controlled player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsPlayerStateLocallyControlled() const;

	/** Returns owner human or bot character.
	 * In blueprints, can be accessed as GetPawn() from base class. */
	class ABmrPawn& GetPawnChecked() const;

	/*********************************************************************************************
	 * Gameplay Ability System (GAS)
	 ********************************************************************************************* */
public:
	/** Returns ability system component that is used to manage abilities and attributes for owned player. */
	virtual FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAbilitySystemComponent& GetAbilitySystemComponentChecked() const;

	/** Returns the gameplay tags owned by this actor from its ASC. */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	/** Initializes all attributes with default values. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void ApplyDefaultAttributes();

protected:
	/** Ability System Component that is used to manage abilities (like place bomb) and attributes (like powerups) for owned player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** Attribute set for powerup-related attributes (powerups pick-up, character stats, etc.).
	 * For read access, can be obtained with UBmrPowerupsAttributeSet::GetPowerupsAttributeSet(Owner) method.
	 * For write access, apply gameplay effects. */
	UPROPERTY(Transient)
	TObjectPtr<class UBmrPowerupsAttributeSet> PowerupsSet = nullptr;

	/** Attribute set for health-related attributes (health, max health, damage).
	 * For read access, can be obtained with UBmrHealthAttributeSet::GetHealthAttributeSet(Owner) method.
	 * For write access, apply gameplay effects. */
	UPROPERTY(Transient)
	TObjectPtr<class UBmrHealthAttributeSet> HealthSet = nullptr;

	/*********************************************************************************************
	 * Chosen Mesh Data
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChosenMeshDataChanged, const FBmrMeshData&, NewMeshData);

	/** Called when player visual choice (character row or skin) changes. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnChosenMeshDataChanged OnChosenMeshDataChanged;

	/** Returns persistent player visual choice (character row + skin). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrMeshData& GetChosenMeshData() const { return ChosenMeshData; }

	/** Returns the Player Tag resolved from the persistent chosen mesh data, survives pawn destruction. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const struct FBmrPlayerTag& GetPlayerTag() const;

	/** Returns true when persistent chosen mesh data is set and points to still an injected row. */ 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	bool IsChosenMeshDataValid() const;

	/** Sets persistent player visual choice. Can be called on server or locally-controlled client (auto-replicates via server RPC). */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (AutoCreateRefTerm = "InMeshData"))
	void SetChosenMeshData(const FBmrMeshData& InMeshData);

protected:
	/** Persistent player visual choice. Replicated so any pool round-trip / dormancy of the pawn keeps the right look. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, ReplicatedUsing = "OnRep_ChosenMeshData", AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FBmrMeshData ChosenMeshData = FBmrMeshData::Empty;

	/** Server RPC, mirrors local SetChosenMeshData call from a locally-controlled client. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "[Bomber]", meta = (BlueprintProtected, AutoCreateRefTerm = "InMeshData"))
	void ServerSetChosenMeshData(const FBmrMeshData& InMeshData);

	/** Called on client when ChosenMeshData replicates. */
	UFUNCTION()
	void OnRep_ChosenMeshData();

	/** Resolves the chosen row on the owned pawn's MapComponent and broadcasts the change. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyChosenMeshData();

	/*********************************************************************************************
	 * End Game State
	 * Is personal for each player: Win, Lose or Draw.
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndGameStateChanged, EBmrEndGameState, EndGameState);

	/** Called when player's match result was changed (Win, lose, draw or none applied). */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnEndGameStateChanged OnEndGameStateChanged;

	/** Returns result of the game for controlled player after ending the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE EBmrEndGameState GetEndGameState() const { return EndGameState; }

	/** Tries to set new End-Game state for this player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void UpdateEndGameState();

	/** Sets End-Game state to the specified one. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetEndGameState(EBmrEndGameState NewEndGameState);

protected:
	/** Contains result of the game for controlled player after ending the game. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_EndGameState", Category = "[Bomber]", meta = (BlueprintProtected))
	EBmrEndGameState EndGameState = EBmrEndGameState::None;

	/** Called on client when End-Game player status is changed. */
	UFUNCTION()
	void OnRep_EndGameState();

	/** Applies currently changed End-Game state for this player. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyEndGameState();

	/***************************************************************************************************************************************
	 * Nickname
	 * - Base::SetPlayerName(Name) for any (Human/AI)
	 * - This::SetDefaultAIName() for AI
	 * - This::SetSavedPlayerName(Name) for human
	 * _____________________________________________________________________________________________________________________________________
	 * | Variable                | Getter Function       | Class | Applies | Cfg | Rep | Description                                       |
	 * |-------------------------|-----------------------|-------|---------|-----|-----|---------------------------------------------------|
	 * | PlayerNamePrivate       | GetPlayerName()       | Base  | Human/AI| -   | +   | Best method to obtain current nickname            |
	 * | OldNamePrivate          | GetPendingPlayerName()| This  | Human   | -   | -   | Pending changed nickname in settings char by char |
	 * | SavedPlayerName		 | GetSavedPlayerName()  | This  | Human   | +   | +   | Saved nickname in game settings                   |
	 **************************************************************************************************************************************/
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, FName, NewName);

	/** Called when player name is changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPlayerNameChanged OnPlayerNameChanged;

	/** Exposes Base::SetOldPlayerName() to blueprints to set locally the player name on each nickname change.*/
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetPendingPlayerName(FName NewName) { SetOldPlayerName(NewName.ToString()); }

	/** Is created on expose code-only GetOldPlayerName() base method to blueprints to get locally the player name on each nickname change. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FName GetPendingPlayerName() const;

	/** Sets saved human name to config property.
	 * SaveConfig() needs to be called separately to save it to the file. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SetSavedPlayerName(FName NewName);

	/** Returns saved human name from config file. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE FName GetSavedPlayerName() const { return SavedPlayerName; }

	/** Attempts to assign default nickname.
	 * - Bots names rely on current character ID like "AI 0", "AI 1" etc.
	 * - Human names are obtained from the OS or online subsystem if available. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetDefaultPlayerName();

	/** Is overridden to additionally set player name on server and broadcast it. */
	virtual void SetPlayerName(const FString& NewPlayerName) override;

	/** Applies and broadcasts player name. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void ApplyPlayerName();

protected:
	/** Is created for saving purposes since base APlayerState::PlayerNamePrivate property is not 'Config'.
	 * Can contain different languages, uppercase, lowercase etc, is config property. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	FName SavedPlayerName;

	/** Called on server when settings are saved to apply local player name. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ServerSetPlayerName(FName NewName);

	/** Called on client when player name is changed. */
	virtual void OnRep_PlayerName() override;

	/*********************************************************************************************
	 * Is Player Dead
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeadChanged, bool, bIsDead);

	/** Called when character dead status is changed: character was killed or revived. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPlayerDeadChanged OnPlayerDeadChanged;

	/** Returns true if current player is alive.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE bool IsPlayerDead() const { return bIsPlayerDead; }

	/** Sets character dead status, true if was killed, false if was revived. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetPlayerDead(bool bIsDead);

protected:
	/** Is true when player is dead. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_IsPlayerDead", AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	bool bIsPlayerDead = false;

	/** Called on client when character Dead status is changed. */
	UFUNCTION()
	void OnRep_IsPlayerDead();

	/** Applies and broadcasts Is Character Dead status. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyIsPlayerDead();

	/** Is called at the end of frame when this character received dead status. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostPlayerDead();

	/*********************************************************************************************
	 * Killed Opponents Num
	 * Is opposite to IsPlayerDead, is increased when this player kills an opponent.
	 *********************************************************************************************/
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpponentsKilledNumChanged, int32, OpponentsKilledNum);

	/** Called when an opponent is killed. May be triggered multiple times during multi-kill increments.
	 * Broadcast only number of killed opponents, might be useful for scoreboards.
	 * To track who exactly killed the player, use delegates like UBmrMapComponent::OnDeactivatedMapComponent. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnOpponentsKilledNumChanged OnOpponentsKilledNumChanged;

	/** Returns the number of opponents killed by the player associated with this Player State. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetOpponentsKilledNum() const { return OpponentsKilledNum; }

	/** Increments the number of opponents killed, is server-only.
	 * @param DefeatedPlayer The character of the opponent who was killed by the player associated with this Player State. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetOpponentKilled(const class ABmrPawn* DefeatedPlayer);

	/** Sets the number of opponents killed by the player associated with this Player State, is server-only.
	 * @param NewOpponentsKilledNum The new number of total opponents killed. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetOpponentKilledNum(int32 NewOpponentsKilledNum);

protected:
	/** Holds the number of opponents killed */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_OpponentsKilledNum", AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	int32 OpponentsKilledNum = 0;

	/** Called on client when Opponents Killed Num changes */
	UFUNCTION()
	void OnRep_OpponentsKilledNum();

	/** Applies and broadcasts Opponents Killed Num changes */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyOpponentsKilledNum();

	/*********************************************************************************************
	 * Is Human / Bot
	 * APlayerState::bIsABot is used to determine if the player is a bot.
	 * - SetIsABot() to assign bot status
	 * - SetIsHuman() to assign human status
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerTypeChanged, EBmrPlayerType, PlayerType);

	/** Called when player is changed from human to bot or vice versa. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPlayerTypeChanged OnPlayerTypeChanged;

	/** Returns the type of owner: Human or AI. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE EBmrPlayerType GetPlayerType() const { return IsABot() ? EBmrPlayerType::Bot : EBmrPlayerType::Human; }

	/** Applies bot status, overloads engine's APlayerState::SetIsABot(bool) that is not virtual and not exposed to blueprints. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetIsABot();

	/** Applies human status. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetIsHuman();

protected:
	/** Called on client when APlayerState::bIsABot is changed. */
	UFUNCTION()
	void OnRep_IsABot();

	/** Applies and broadcasts IsABot status. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyIsABot();

	/*********************************************************************************************
	 * Player ID (0, 1, 2, 3)
	 * APlayerState::PlayerId is used to determine the player's ID.
	 * - SetHumanId(PlayerController) to assign human ID
	 * - SetDefaultBotId() to assign bot ID
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerIdChanged, int32, PlayerId);

	/** Called when player ID is changed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "[Bomber]")
	FOnPlayerIdChanged OnPlayerIdChanged;

	/** Applies ID from order of player controllers, is always 0, 1, 2, 3. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetHumanId(class APlayerController* PlayerController);

	/** Applies ID from order of spawned characters on level, is always 0, 1, 2, 3. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetDefaultBotId();

protected:
	/** Called on client when player ID is changed. */
	virtual void OnRep_PlayerId() override;

	/** Applies and broadcasts player ID. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyPlayerId();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	/** Is called when player state is initialized with assigned character.
	 * Can be called multiple times on each player join due to reusing player states. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]")
	void OnPlayerStateInit();

	/** Is called on server and clients when new owned pawn is possessed or changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]")
	void OnPawnChanged(class APawn* NewPawn);

	/** Listens game states to notify server about ending game for controlled player. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnGameStateChanged(const struct FGameplayEventData& Payload);

	/** Listens game settings to apply them once saved. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnSaveSettings();

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

	/** Is overridden to prevent the player state from being destroyed to be able to reuse it by bots. */
	virtual void OnDeactivated() override;

	/** Register a player with the online subsystem. */
	virtual void RegisterPlayerWithSession(bool bWasFromInvite) override;

	/** Unregister a player with the online subsystem. */
	virtual void UnregisterPlayerWithSession() override;

	/** Is overridden to handle own OnRep functions for engine properties.
	 * Called right after calling all OnRep notifies (called even when there are no notifies). */
	virtual void PostRepNotifies() override;
};