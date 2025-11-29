// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Pawn.h"

// UE
#include "AbilitySystemInterface.h"

#include "PlayerCharacter.generated.h"

enum class ELevelType : uint8;
enum class ECurrentGameState : uint8;
enum class EPlayerType : uint8;

/**
 * Players and AI, whose goal is to remain the last survivor for the win.
 * @see Access Player's data with UPlayerDataAsset (Content/Bomber/DataAssets/DA_Player).
 * @see Access AI's data with UAIDataAsset (Content/Bomber/DataAssets/DA_AI).
 */
UCLASS(Abstract)
class BOMBER_API APlayerCharacter : public APawn,
                                    public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	/** Sets default values for this character's properties */
	APlayerCharacter();

	/** Returns the Player Tag associated with player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const struct FPlayerTag& GetPlayerTag() const;

	/** Returns the Ability System Component from the Player State.
	 * In blueprints, call 'Get Ability System Component' as interface function. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAbilitySystemComponent& GetAbilitySystemComponentChecked() const;

protected:
	/** Is the root component for this actor, used for collision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Capsule Component"))
	TObjectPtr<class UCapsuleComponent> CapsuleComponentInternal = nullptr;

	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);

	/** Is called on server when human player, previously possessed by this character, left the session. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostLogout(class APlayerController* ExitingPlayer);

protected:
	/** Called when this level actor is reconstructed or added on the Generated Map.
	 * Is used by Level Actors instead of the BeginPlay(). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnAddedToLevel(UMapComponent* MapComponent);

	/** Is called when the Row from current Data Asset is changed for owner on the level, on both server and clients. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnActorTypeChanged(UMapComponent* MapComponent, const class ULevelActorRow* NewRow, const class ULevelActorRow* PreviousRow);

	/** Called right before owner actor going to remove from the Generated Map, on both server and clients.
	 * Is used for handling the in-game dying logic before this character is removed from the level. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPreRemovedFromLevel(class UMapComponent* MapComponent, UObject* DestroyCauser);

	/** Called each time after owner actor was removed from Generated Map, on both server and clients.
	 * Is used for cleaning up the character's data after it was removed from the level. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostRemovedFromLevel(class UMapComponent* MapComponent, UObject* DestroyCauser);

	/** Is called for everytime when character changed its position on the Generated Map. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCellChanged(class UMapComponent* MapComponent, const struct FCell& NewCell, const struct FCell& PreviousCell);

	/** Is called when the player state is fully initialized. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerStateReady(class AMyPlayerState* InPlayerState, int32 CharacterID);

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Updates collision object type by current character ID. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateCollisionObjectType();

	/** Sets current config: each character has its own configuration, like different starting attributes. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void ApplyCharacterConfig();

	/*********************************************************************************************
	 * Controller (AI/Player)
	 ********************************************************************************************* */
public:
	/** Is overridden to determine additional conditions for the player-controlled character. */
	virtual bool IsPlayerControlled() const override;

	/** Possess a player or AI controller in dependence of current Character ID.
	 * @param PlayerType the type of player to possess: human or bot; or 'Any' to automatically detect.*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void TryPossessController(EPlayerType PlayerType);

protected:
	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	TObjectPtr<class AAIController> AIControllerInternal = nullptr;

	/*********************************************************************************************
	 * Movement
	 ********************************************************************************************* */
public:
	/** Is overridden to return the velocity from the Mover Component instead. */
	virtual FVector GetVelocity() const override;

	/** Returns the movement component for the player character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UBmrMoverComponent* GetMoverComponent() const { return MoverComponentInternal; }

protected:
	/** Movement component for the player character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mover Component"))
	TObjectPtr<class UBmrMoverComponent> MoverComponentInternal = nullptr;

	/*********************************************************************************************
	 * Nickname
	 ********************************************************************************************* */
public:
	/** Returns the 3D widget component that displays the player name above the character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UBmrPlayerNameWidgetComponent* GetPlayerName3DWidgetComponent() const { return PlayerName3DWidgetComponentInternal; }

protected:
	/** 3D widget component that displays the player name above the character. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Name 3D Widget Component"))
	TObjectPtr<class UBmrPlayerNameWidgetComponent> PlayerName3DWidgetComponentInternal = nullptr;

	/*********************************************************************************************
	 * Player ID
	 ********************************************************************************************* */
public:
	/** Returns own character ID, e.g: 0, 1, 2, 3 */
	UFUNCTION(BlueprintPure, Category = "C++")
	int32 GetPlayerId() const;

	/*********************************************************************************************
	 * Player Mesh
	 ********************************************************************************************* */
public:
	friend class UMyCheatManager;

	/** Returns owned skeletal mesh component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UMySkeletalMeshComponent* GetMeshComponent() const { return MeshComponentInternal; }

	/** Returns owned skeletal mesh component, or crashes if can't be obtained. */
	UMySkeletalMeshComponent& GetMeshComponentChecked() const;

	/** Set and apply default skeletal mesh for this player.
	 * @param bForcePlayerSkin If true, will force the bot to change own skin to look like a player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void SetDefaultPlayerMeshData(bool bForcePlayerSkin = false);

protected:
	/** The Skeletal Mesh Component of the player character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Mesh Component"))
	TObjectPtr<class UMySkeletalMeshComponent> MeshComponentInternal = nullptr;

	/*********************************************************************************************
	 * Bomb Placement
	 ********************************************************************************************* */
public:
	/** Spawns bomb on character position. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SpawnBomb();
};