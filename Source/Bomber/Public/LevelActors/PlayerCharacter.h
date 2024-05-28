// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Character.h"
//---
#include "Structures/CustomPlayerMeshData.h"
#include "Structures/PlayerTag.h"
//---
#include "PlayerCharacter.generated.h"

enum class ELevelType : uint8;
enum class ECurrentGameState : uint8;

/**
 * Numbers of power-ups that affect the abilities of a player during gameplay.
 * @todo JanSeliv UGi56jhn Use GAS attributes for picked up items
 */
USTRUCT(BlueprintType, DisplayName = "Power-Ups")
struct BOMBER_API FPowerUp
{
	GENERATED_BODY()

	/** Default amount on picked up items. */
	static const FPowerUp DefaultData;

	/** The number of items, that increases the movement speed of the character */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 SkateN = 1;

	/** Maximum number of bombs that can be put at one time */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 BombN = 1;

	/** Current amount of bombs available.
	 * Decreases with every bomb spawn and increases with every bomb explosion.
	 * Is always less or equal to BombN. */
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Transient, Category = "C++")
	int32 BombNCurrent = 1;

	/** The number of items, that increases the bomb blast radius */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 FireN = 1;
};

/**
 * Players and AI, whose goal is to remain the last survivor for the win.
 * @see Access Player's data with UPlayerDataAsset (Content/Bomber/DataAssets/DA_Player).
 * @see Access AI's data with UAIDataAsset (Content/Bomber/DataAssets/DA_AI).
 * @todo JanSeliv NlwqUwmc Reorder td sections of PlayerCharacter.h
 */
UCLASS()
class BOMBER_API APlayerCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerTypeChanged, FPlayerTag, PlayerTag);

	/** Called when chosen player's mesh changed for this pawn. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPlayerTypeChanged OnPlayerTypeChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerUpsChanged, const struct FPowerUp&, AllPowerUps);

	/** Called when this character picked up any power-up or they were reset. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPowerUpsChanged OnPowerUpsChanged;

	/** ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this character's properties */
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	/** Initialize a player actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructPlayerCharacter();

	/** Returns current powerup levels */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FPowerUp& GetPowerups() const { return PowerupsInternal; }

	/** Spawns bomb on character position */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "C++")
	void ServerSpawnBomb();

	/** Returns the Skeletal Mesh of bombers. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;

	/** Set and apply how a player has to look like.
	 * @param CustomPlayerMeshData New data to apply. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void ServerSetCustomPlayerMeshData(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/** Returns current player mesh data of  the local player applied to skeletal mesh. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

	/** Returns level type associated with player, e.g: Water level type for Roger character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetPlayerType() const;

	/** Returns the Player Tag associated with player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FGameplayTag& GetPlayerTag() const;

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The MapComponent manages this actor on the Generated Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr;

	/** Count of items that affect on a player during gameplay. Can be overriden by the Cheat Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_Powerups", Category = "C++", meta = (BlueprintProtected, DisplayName = "Powerups", ShowOnlyInnerProperties))
	FPowerUp PowerupsInternal = FPowerUp::DefaultData;

	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	TObjectPtr<class AMyAIController> MyAIControllerInternal = nullptr;

	/** Contains custom data about mesh tweaked by player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_PlayerMeshData", Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal = FCustomPlayerMeshData::Empty;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Is called on a player character construction, could be called multiple times.
	 * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructPlayerCharacter() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionPlayerCharacter().
	 * @warning Do not call directly, use ThisClass::ConstructPlayerCharacter() instead. */
	UFUNCTION()
	void OnConstructionPlayerCharacter();

	/** Called every frame, is disabled on start, tick interval is decreased. */
	virtual void Tick(float DeltaTime) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Is overriden to handle the client login when is set new player state. */
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Called when this Pawn is possessed. Only called on the server (or in standalone).
	 * @param NewController The controller possessing this pawn. */
	virtual void PossessedBy(AController* NewController) override;

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/**
	 * Triggers when this player character starts something overlap.
	 * With item overlapping Increases +1 to numbers of character's powerups (Skate/Bomb/Fire).
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Event triggered when the bomb has been explicitly destroyed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombDestroyed(class UMapComponent* MapComponent, UObject* DestroyCauser = nullptr);

	/** Listen to manage the tick. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Apply effect of picked up powerups. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyPowerups();

	/** Reset all picked up powerups. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ResetPowerups();

	/** Is called on clients to apply powerups for this character. */
	UFUNCTION()
	void OnRep_Powerups();

	/** Updates collision object type by current character ID. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateCollisionObjectType();

	/** Possess a player or AI controller in dependence of current Character ID. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void TryPossessController();

	/** Is called on game mode post login to handle character logic when new player is connected. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);

	/** Set and apply new skeletal mesh from current data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCustomPlayerMeshData();

	/** Set and apply default skeletal mesh for this player. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SetDefaultPlayerMeshData();

	/** Respond on changes in player mesh data to update the mesh on client. */
	UFUNCTION()
	void OnRep_PlayerMeshData();

	/** Move the player character. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ActionValue"))
	void MovePlayer(const struct FInputActionValue& ActionValue);

	/** Is called when the player was destroyed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerRemovedFromLevel(UMapComponent* MapComponent, UObject* DestroyCauser);

	/*********************************************************************************************
	 * Nickname
	 ********************************************************************************************* */
public:
	/** Updates new player name on a 3D widget component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetNicknameOnNameplate(FName NewName);

protected:
	/** The static mesh nameplate (background material of the player name).
	 * @todo JanSeliv whnin60J Get rid of `Nameplate Mesh` from Player Character: use Image background image in PlayerName3DWidget instead. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nameplate Mesh Component"))
	TObjectPtr<class UStaticMeshComponent> NameplateMeshInternal = nullptr;

	/** 3D widget component that displays the player name above the character. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Name 3D Widget Component"))
	TObjectPtr<class UWidgetComponent> PlayerName3DWidgetComponentInternal = nullptr;

	/*********************************************************************************************
	 * Player ID
	 ********************************************************************************************* */
public:
	/** Returns own character ID, e.g: 0, 1, 2, 3 */
	UFUNCTION(BlueprintPure, Category = "C++")
	int32 GetPlayerId() const;

	/** Applies the playerID-dependent logic for this character. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ApplyPlayerId();

protected:
	/** The ID identification of each character.
	 * @todo JanSeliv 4ZkNWtmN Replace `APlayerCharacter::CharacterID` by `APlayerState::PlayerId`. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CharacterID", Category = "C++", meta = (BlueprintProtected, DisplayName = "Character ID"))
	int32 CharacterIDInternal = INDEX_NONE;

	/** Is called on clients to apply the characterID-dependent logic for this character. */
	UFUNCTION()
	void OnRep_CharacterID();
};