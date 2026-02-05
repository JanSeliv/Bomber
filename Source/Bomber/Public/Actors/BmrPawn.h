// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Pawn.h"

// UE
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"

#include "BmrPawn.generated.h"

enum class EBmrLevelType : uint8;

enum class EBmrPlayerType : uint8;

/**
 * Players and AI, whose goal is to remain the last survivor for the win.
 * @see Access Player's data with UBmrPlayerDataAsset (Content/Bomber/DataAssets/DA_Player).
 * @see Access AI's data with UBmrAIDataAsset (Content/Bomber/DataAssets/DA_AI).
 */
UCLASS(Abstract)
class BOMBER_API ABmrPawn : public APawn,
                            public IAbilitySystemInterface,
                            public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	/** Sets default values for this character's properties */
	ABmrPawn();

	/** Returns the Player Tag associated with player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const struct FBmrPlayerTag& GetPlayerTag() const;

protected:
	/** Is the root component for this actor, used for collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UCapsuleComponent> CapsuleComponent = nullptr;

	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMapComponent> MapComponent = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
public:
	/** Returns the Ability System Component from the Player State.
	 * In blueprints, call 'Get Ability System Component' as interface function. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAbilitySystemComponent& GetAbilitySystemComponentChecked() const;

	/** Returns the gameplay tags owned by this actor, delegates to PlayerState's ASC. */
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is overriden to handle the client login when is set new player state. */
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	/** Is called on server when ANY human player joined the session. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);

	/** Is called on server when human player, previously possessed by this character, left the session. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostLogout(class APlayerController* ExitingPlayer);

protected:
	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnAddedToLevel(UBmrMapComponent* InMapComponent);

	/** Is called when the Row from current Data Asset is changed for owner on the level, on both server and clients. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnActorTypeChanged(UBmrMapComponent* InMapComponent, const class UBmrLevelActorRow* NewRow, const class UBmrLevelActorRow* PreviousRow);

	/** Called right before owner actor going to remove from the Generated Map, on both server and clients.
	 * Is used for handling the in-game dying logic before this character is removed from the level. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPreRemovedFromLevel(UBmrMapComponent* InMapComponent, UObject* DestroyCauser);

	/** Called each time after owner actor was removed from Generated Map, on both server and clients.
	 * Is used for cleaning up the character's data after it was removed from the level. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(UBmrMapComponent* InMapComponent, UObject* DestroyCauser);

	/** Is called for everytime when character changed its position on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnCellChanged(UBmrMapComponent* InMapComponent, const struct FBmrCell& NewCell, const struct FBmrCell& PreviousCell);

	/** Is called when the pawn is fully initialized and ready */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void OnPawnReady(const struct FGameplayEventData& Payload);

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Updates collision object type by current character ID. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]", meta = (BlueprintProtected))
	void UpdateCollisionObjectType();

	/** Sets current config: each character has its own configuration, like different starting attributes. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	void ApplyPreset();

	/*********************************************************************************************
	 * Controller (AI/Player)
	 ********************************************************************************************* */
public:
	/** Is overridden to determine additional conditions for the player-controlled character. */
	virtual bool IsPlayerControlled() const override;

	/** Possess a player or AI controller in dependence of current Character ID.
	 * @param PlayerType the type of player to possess: human or bot; or 'Any' to automatically detect.*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void TryPossessController(EBmrPlayerType PlayerType);

protected:
	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class AAIController> AIController = nullptr;

	/*********************************************************************************************
	 * Movement
	 ********************************************************************************************* */
public:
	/** Is overridden to return the velocity from the Mover Component instead. */
	virtual FVector GetVelocity() const override;

	/** Returns the movement component for the player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBmrMoverComponent* GetMoverComponent() const { return MoverComponent; }

protected:
	/** Movement component for the player character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrMoverComponent> MoverComponent = nullptr;

	/*********************************************************************************************
	 * UI
	 ********************************************************************************************* */
public:
	/** Returns the 3D widget component that displays the player name above the character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBmrPlayerNameWidgetComponent* GetPlayerName3DWidgetComponent() const { return PlayerName3DWidgetComponent; }

	/** Returns static mesh component that displays the arrow above the local player during match start. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBmrPlayerArrowStartComponent* GetPlayerArrowStartWidgetComponent() const { return PlayerArrowStartComponent; }

protected:
	/** 3D widget component that displays the player name above the character. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrPlayerNameWidgetComponent> PlayerName3DWidgetComponent = nullptr;

	/** Static mesh component that displays the arrow above the local player during match start. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrPlayerArrowStartComponent> PlayerArrowStartComponent = nullptr;

	/*********************************************************************************************
	 * Player ID
	 ********************************************************************************************* */
public:
	/** Returns own character ID, e.g: 0, 1, 2, 3 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetPlayerId() const;

	/*********************************************************************************************
	 * Player Mesh
	 ********************************************************************************************* */
public:
	friend class UBmrCheatManager;

	/** Returns owned skeletal mesh component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE class UBmrSkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; }

	/** Returns owned skeletal mesh component, or crashes if can't be obtained. */
	UBmrSkeletalMeshComponent& GetMeshComponentChecked() const;

	/** Set and apply default skeletal mesh for this player.
	 * @param bForcePlayerSkin If true, will force the bot to change own skin to look like a player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "[Bomber]")
	void SetDefaultPlayerMeshData(bool bForcePlayerSkin = false);

protected:
	/** The Skeletal Mesh Component of the player character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "[Bomber]", meta = (BlueprintProtected))
	TObjectPtr<class UBmrSkeletalMeshComponent> MeshComponent = nullptr;

	/*********************************************************************************************
	 * Bomb Placement
	 ********************************************************************************************* */
public:
	/** Spawns bomb on character position. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void SpawnBomb();
};