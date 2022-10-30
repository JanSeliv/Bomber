// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/Character.h"
//---
#include "InputActionValue.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Globals/LevelActorDataAsset.h"
//---
#include "PlayerCharacter.generated.h"

/**
 * Determines each mesh to attach.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FAttachedMesh
{
	GENERATED_BODY()

	/** The attached static mesh or skeletal mesh.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties, ExposeOnSpawn))
	TObjectPtr<class UStreamableRenderAsset> AttachedMesh = nullptr; //[D]

	/** In the which socket should attach this prop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FName Socket = NAME_None; //[D]

	/** Prop animation is loop played all the time, starts playing on attaching to the owner. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> MeshAnimation = nullptr; //[D]
};

/**
 * The player archetype of level actor rows. Determines the individual of the character model
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerRow final : public ULevelActorRow
{
	GENERATED_BODY()

public:
	/** All meshes that will be attached to the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TArray<FAttachedMesh> PlayerProps; //[D]

	/** The own movement animation for the each character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UBlendSpace1D> IdleWalkRunBlendSpace = nullptr; //[D]

	/** Dance animation that is used mostly in the menu instead of idle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row", meta = (ShowOnlyInnerProperties))
	TObjectPtr<class UAnimSequence> DanceAnimation = nullptr; //[D]

	/** Returns the num of skin textures in the array of diffuse maps specified a player material instance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetMaterialInstancesDynamicNum() const { return MaterialInstancesDynamicInternal.Num(); }

	/** Returns the dynamic material instance of a player with specified skin.
	 * @param SkinIndex The skin position to get.
	 * @see UPlayerRow::MaterialInstancesDynamicInternal */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMaterialInstanceDynamic* GetMaterialInstanceDynamic(int32 SkinIndex) const;

protected:
	/** The material instance of a player.
	 * @warning Is not BlueprintReadOnly and has not getter to prevent being used directly, we have dynamic materials instead.
	 * @see UPlayerRow::MaterialInstancesDynamicInternal. */
	UPROPERTY(EditDefaultsOnly, Category = "Row", meta = (BlueprintProtected, DisplayName = "Material Instance", ShowOnlyInnerProperties))
	TObjectPtr<class UMaterialInstance> MaterialInstanceInternal = nullptr; //[D]

	/**
	 * Contains all created dynamic materials for each skin in the Material Instance.
	 * Saves memory avoiding creation of dynamic materials for each mesh component, just use the same dynamic material for different meshes with the same skin.
	 * Is filled on object creating and changing.
	 * @warning Is not EditDefaultsOnly because have to be created dynamically, in the same time is incompatible with VisibleInstanceOnly.
	 * @see UPlayerRow::MaterialInstanceInternal. */
	UPROPERTY(BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Material Instances Dynamic"))
	TArray<TObjectPtr<class UMaterialInstanceDynamic>> MaterialInstancesDynamicInternal; //[G]

#if WITH_EDITOR
	/** Handle adding and changing material instance to prepare dynamic materials. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Create dynamic material instance for each ski if is not done before.
	 * UPlayerRow::MaterialInstancesDynamicInternal
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void TryCreateDynamicMaterials();
#endif	//WITH_EDITOR
};

/**
 * The data asset of the Bomber characters
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UPlayerDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UPlayerDataAsset();

	/** Returns the player data asset. */
	static const UPlayerDataAsset& Get();

	/** The num of nameplate materials.  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNameplateMaterialsNum() const { return NameplateMaterialsInternal.Num(); }

	/** Returns a nameplate material by index, is used by nameplate meshes.
	 * @see UPlayerDataAsset::NameplateMaterials */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	class UMaterialInterface* GetNameplateMaterial(int32 Index) const;

	/** Returns the Anim Blueprint class to use.
	 * @see UPlayerDataAsset::AnimInstanceClassInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<UAnimInstance> GetAnimInstanceClass() const { return AnimInstanceClassInternal; }

	/** Returns the name of a material parameter with a diffuse array.
	 * @see UPlayerDataAsset::SkinSlotNameInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FName GetSkinArrayParameter() const { return SkinArrayParameterInternal; }

	/** Returns the name of a material parameter with a diffuse index.
	* @see UPlayerDataAsset::SkinSlotNameInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FName GetSkinIndexParameter() const { return SkinIndexParameterInternal; }

protected:
	/** All materials that are used by nameplate meshes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Nameplate Materials", ShowOnlyInnerProperties))
	TArray<TObjectPtr<class UMaterialInterface>> NameplateMaterialsInternal; //[D]

	/** The AnimBlueprint class to use, can set it only in the gameplay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Anim Instance Class", ShowOnlyInnerProperties))
	TSubclassOf<UAnimInstance> AnimInstanceClassInternal = nullptr; //[D]

	/** The name of a material parameter with a diffuse array. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skin Array Parameter", ShowOnlyInnerProperties))
	FName SkinArrayParameterInternal = TEXT("DiffuseArray"); //[D]

	/** The name of a material parameter with a diffuse index. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Skin Index Parameter", ShowOnlyInnerProperties))
	FName SkinIndexParameterInternal = TEXT("DiffuseIndex"); //[D]
};

/**
 * Numbers of power-ups that affect the abilities of a player during gameplay.
 * @todo rewrite as attributes of ability system
 */
USTRUCT(BlueprintType)
struct BOMBER_API FPowerUp
{
	GENERATED_BODY()

	/** Default amount on picked up items. */
	static const FPowerUp DefaultData;

	/** The number of items, that increases the movement speed of the character */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 SkateN = 1;

	/** The number of bombs that can be set at one time */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 BombN = 1;

	/** The number of items, that increases the bomb blast radius */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 FireN = 1;
};

/**
 * Players and AI, whose goal is to remain the last survivor for the win.
 */
UCLASS()
class BOMBER_API APlayerCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	/** ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this character's properties */
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	/** Returns current powerup levels */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FPowerUp& GetPowerups() const { return PowerupsInternal; }

	/** Returns the personal ID. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetCharacterID() const { return CharacterIDInternal; }

	/** Spawns bomb on character position */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "C++")
	void ServerSpawnBomb();

	/** Returns the Skeletal Mesh of bombers. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const { return Cast<UMySkeletalMeshComponent>(GetMesh()); }

	/** Actualize the player name for this character. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateNicknameOnNameplate();

	/** Set and apply how a player has to look like.
	 * @param CustomPlayerMeshData New data to apply. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "C++", meta = (AutoCreateRefTerm = "CustomPlayerMeshData"))
	void ServerSetCustomPlayerMeshData(const FCustomPlayerMeshData& CustomPlayerMeshData);

	/** Returns current player mesh data of  the local player applied to skeletal mesh. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const FORCEINLINE FCustomPlayerMeshData& GetCustomPlayerMeshData() const { return PlayerMeshDataInternal; }

protected:
	/** ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	TObjectPtr<class UMapComponent> MapComponentInternal = nullptr; //[C.AW]

	/** The static mesh nameplate */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nameplate Mesh Component"))
	TObjectPtr<class UStaticMeshComponent> NameplateMeshInternal = nullptr; //[C.DO]

	/** Count of items that affect on a player during gameplay. Can be overriden by the Cheat Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_Powerups", Category = "C++", meta = (BlueprintProtected, DisplayName = "Powerups", ShowOnlyInnerProperties))
	FPowerUp PowerupsInternal = FPowerUp::DefaultData; //[AW]

	/** The ID identification of each character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_CharacterID", Category = "C++", meta = (BlueprintProtected, DisplayName = "Character ID"))
	int32 CharacterIDInternal = INDEX_NONE; //[G]

	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	TObjectPtr<class AMyAIController> MyAIControllerInternal = nullptr; //[G]

	/** Contains custom data about mesh tweaked by player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, ReplicatedUsing = "OnRep_PlayerMeshData", Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Mesh Data"))
	FCustomPlayerMeshData PlayerMeshDataInternal = FCustomPlayerMeshData::Empty; //[G]

	/** ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called every frame, is disabled on start, tick interval is decreased. */
	virtual void Tick(float DeltaTime) override;

	/** Returns properties that are replicated for the lifetime of the actor channel. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Is overriden to handle the client login when is set new player state. */
	virtual void OnRep_PlayerState() override;

	/** Sets the actor to be hidden in the game. Alternatively used to avoid destroying. */
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	/** Called when this Pawn is possessed. Only called on the server (or in standalone).
	 * @param NewController The controller possessing this pawn. */
	virtual void PossessedBy(AController* NewController) override;

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

	/** Updates new player name on a 3D widget component. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void SetNicknameOnNameplate(FName NewName);

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

	/** Apply the characterID-dependent logic for this character. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void ApplyCharacterID();

	/** Is called on clients to apply the characterID-dependent logic for this character. */
	UFUNCTION()
	void OnRep_CharacterID();

	/** Move the player character by the forward vector. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ActionValue"))
	void MoveBackForward(const FInputActionValue& ActionValue);

	/** Move the player character by the right vector. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected, AutoCreateRefTerm = "ActionValue"))
	void MoveRightLeft(const FInputActionValue& ActionValue);
};
