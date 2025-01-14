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

	/** Operator to set all values at once from one integer. */
	FPowerUp& operator=(int32 NewValue);
};

/**
 * Players and AI, whose goal is to remain the last survivor for the win.
 * @see Access Player's data with UPlayerDataAsset (Content/Bomber/DataAssets/DA_Player).
 * @see Access AI's data with UAIDataAsset (Content/Bomber/DataAssets/DA_AI).
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

	/*********************************************************************************************
	 * Powerups
	 ********************************************************************************************* */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerUpsChanged, const struct FPowerUp&, AllPowerUps);

	/** Called when this character picked up any power-up or they were reset. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "C++")
	FOnPowerUpsChanged OnPowerUpsChanged;

	/** Returns current powerup levels */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FPowerUp& GetPowerups() const { return PowerupsInternal; }

	/** Set powerups levels all at once. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (AutoCreateRefTerm = "Powerups"))
	void SetPowerups(int32 NewLevel);

	/** Apply effect of picked up powerups. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyPowerups();

	/** Reset all picked up powerups. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ResetPowerups();

protected:
	/** Count of items that affect on a player during gameplay. Can be overriden by the Cheat Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_Powerups", Category = "C++", meta = (BlueprintProtected, DisplayName = "Powerups", ShowOnlyInnerProperties))
	FPowerUp PowerupsInternal = FPowerUp::DefaultData;

	/** Is called on clients to apply powerups for this character. */
	UFUNCTION()
	void OnRep_Powerups();

	/** ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */
public:
	/** Sets default values for this character's properties */
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	/** Initialize a player actor, could be called multiple times. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ConstructPlayerCharacter();

	/** Returns level type associated with player, e.g: Water level type for Roger character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	ELevelType GetPlayerType() const;

	/** Returns the Player Tag associated with player. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FGameplayTag& GetPlayerTag() const;

protected:
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

	/** Called every frame, is disabled on start, tick interval is decreased. */
	virtual void Tick(float DeltaTime) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Is overriden to handle the client login when is set new player state. */
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Is called on a player character construction, could be called multiple times.
	 * Could be listened by binding to UMapComponent::OnOwnerWantsReconstruct delegate.
	 * See the call stack below for more details:
	 * AActor::RerunConstructionScripts() -> AActor::OnConstruction() -> ThisClass::ConstructPlayerCharacter() -> UMapComponent::ConstructOwnerActor() -> ThisClass::OnConstructionPlayerCharacter().
	 * @warning Do not call directly, use ThisClass::ConstructPlayerCharacter() instead. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnConstructionPlayerCharacter();

	/**
	 * Triggers when this player character starts something overlap.
	 * With item overlapping Increases +1 to numbers of character's powerups (Skate/Bomb/Fire).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Listen to manage the tick. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Is called on game mode post login to handle character logic when new player is connected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);

	/** Is called when the player was destroyed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerRemovedFromLevel(UMapComponent* MapComponent, UObject* DestroyCauser);

	/** Is called when the player character is fully initialized. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnCharacterReady(APlayerCharacter* Character, int32 CharacterID);

	/** Is called when all game widgets are initialized to handle UI-related logic.
	 * Is not called on remote clients. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnWidgetsInitialized();

	/*********************************************************************************************
	 * Protected functions
	 ********************************************************************************************* */
protected:
	/** Updates collision object type by current character ID. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateCollisionObjectType();

	/*********************************************************************************************
	 * Controller (AI/Player)
	 ********************************************************************************************* */
public:
	/** Is overridden to determine additional conditions for the player-controlled character. */
	virtual bool IsPlayerControlled() const override;

	/** Possess a player or AI controller in dependence of current Character ID. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++")
	void TryPossessController();

	/** Move the player character. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ActionValue"))
	void MovePlayer(const struct FInputActionValue& ActionValue);

protected:
	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	TObjectPtr<class AAIController> AIControllerInternal = nullptr;

	/** Called when this Pawn is possessed. Only called on the server (or in standalone).
	 * @param NewController The controller possessing this pawn. */
	virtual void PossessedBy(AController* NewController) override;

	/*********************************************************************************************
	 * Nickname
	 ********************************************************************************************* */
public:
	/** Updates new player name on a 3D widget component. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetNicknameOnNameplate(FName NewName);

	/** Returns the static mesh nameplate (background material of the player name). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UStaticMeshComponent* GetNameplateMesh() const { return NameplateMeshInternal; }

	/** Returns the 3D widget component that displays the player name above the character. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE class UWidgetComponent* GetPlayerName3DWidgetComponent() const { return PlayerName3DWidgetComponentInternal; }

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

protected:
	/** Applies the playerID-dependent logic for this character. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ApplyPlayerId(int32 CurrentPlayerId = -1);

	/*********************************************************************************************
	 * Player Mesh
	 ********************************************************************************************* */
public:
	friend class UMyCheatManager;

	/** Returns the Skeletal Mesh of bombers. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;

	/** Returns current player mesh data of  the local player applied to skeletal mesh. */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

	/** Set and apply how a player has to look like.
	 * It will call Server RPC if called on the client.
	 * @param CustomPlayerMeshData New data to apply. May accept just tag from its constructor, in BP use MakeCustomPlayerMeshData. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void SetCustomPlayerMeshData(const FCustomPlayerMeshData& CustomPlayerMeshData);

protected:
	/** Contains custom data about mesh tweaked by player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_PlayerMeshData", AdvancedDisplay, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal = FCustomPlayerMeshData::Empty;

	/** Server RPC to set and apply how a player has to look like.
	 * @param CustomPlayerMeshData New data to apply. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "CustomPlayerMeshData"))
	void ServerSetCustomPlayerMeshData(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/** Set and apply new skeletal mesh from current data. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCustomPlayerMeshData();

	/** Set and apply default skeletal mesh for this player.
	 * @param bForcePlayerSkin If true, will force the bot to change own skin to look like a player. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "C++", meta = (BlueprintProtected))
	void SetDefaultPlayerMeshData(bool bForcePlayerSkin = false);

	/** Respond on changes in player mesh data to update the mesh on client. */
	UFUNCTION()
	void OnRep_PlayerMeshData();

	/*********************************************************************************************
	 * Bomb Placement
	 ********************************************************************************************* */
public:
	/** Spawns bomb on character position */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "C++")
	void ServerSpawnBomb();

protected:
	/** Event triggered when the bomb has been explicitly destroyed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombDestroyed(class UMapComponent* MapComponent, UObject* DestroyCauser = nullptr);
};