// Copyright 2020 Yevhenii Selivanov.

#pragma once

#include "Globals/LevelActorDataAsset.h"
//---
#include "GameFramework/Character.h"
//---
#include "PlayerCharacter.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UPlayerRow final : public ULevelActorRow
{
	GENERATED_BODY()

public:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	class UBlendSpace1D* IdleWalkRunBlendSpace; //[D]

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Row")
	class UAnimSequence* DanceAnimation; //[D]
};

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class UPlayerDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UPlayerDataAsset();

	/** All materials that are used by nameplate meshes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TArray<class UMaterialInterface*> NameplateMaterials; //[M.DO]

	/* The AnimBlueprint class to use, can set it only in the gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	class TSubclassOf<UAnimInstance> AnimBlueprintClass;
};

/**
 * Numbers of power-ups that affect the abilities of a player during gameplay.
 * @todo rewrite as attributes of ability system
 */
USTRUCT(BlueprintType)
struct FPowerUp
{
	GENERATED_BODY()

	/** Empty constructor */
	FPowerUp()
	{
	};

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
	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this character's properties */
	APlayerCharacter();

	/** */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE FPowerUp GetPowerups() const { return PowerupsInternal; }

	/** Returns the personal ID. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetCharacterID() const { return CharacterIDInternal; }

	/**  Finds and rotates the self at the current character's location to point at the specified location.
	 * @param Location the character is looking at.
	 * @param bShouldInterpolate if true, smoothly rotate the character toward the direction. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "C++", meta = (AutoCreateRefTerm = "Location"))
	void RotateToLocation(const FVector& Location, bool bShouldInterpolate) const;

	/** Spawns bomb on character position */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SpawnBomb();

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */

	friend class UMyCheatManager;

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Map Component"))
	class UMapComponent* MapComponentInternal; //[C.AW]

	/** The static mesh nameplate */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "Nameplate Mesh Component"))
	class UStaticMeshComponent* NameplateMeshInternal; //[C.DO]

	/** Count of items that affect on a player during gameplay. Can be overriden by the Cheat Manager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Powerups", ShowOnlyInnerProperties))
	FPowerUp PowerupsInternal; //[AW]

	/** The ID identification of each character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Character ID"))
	int32 CharacterIDInternal = INDEX_NONE; //[G]

	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "My AI Controller"))
	class AMyAIController* MyAIControllerInternal; //[G]

	/** Store to pause and unpause updating the current location of the player and bots on the level map.  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Update Handle"))
	FTimerHandle UpdatePositionHandleInternal;

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/* Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Virtual overriding of the UFUNCTION.
	 * Adds the movement input along the given world direction vector.
	 *
	 * @param WorldDirection Direction in world space to apply input
	 * @param ScaleValue Scale to apply to input. This can be used for analog input, ie a value of 0.5 applies half the normal value, while -1.0 would reverse the direction.
	 * @param bForce If true always add the input, ignoring the result of IsMoveInputIgnored().
	 */
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.f, bool bForce = false) override;

	/* Move the player character by the forward vector. */
	FORCEINLINE void OnMoveUpDown(float ScaleValue) { AddMovementInput(GetActorForwardVector(), ScaleValue); }

	/* Move the player character by the right vector. */
	FORCEINLINE void OnMoveRightLeft(float ScaleValue) { AddMovementInput(GetActorRightVector(), ScaleValue); }

	/**
	 * Triggers when this player character starts something overlap.
	 * With item overlapping Increases +1 to numbers of character's powerups (Skate/Bomb/Fire).
	 */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	/** Event triggered when the bomb has been explicitly destroyed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnBombDestroyed(AActor* DestroyedBomb);

	/** */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/**  */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure = false, Category = "C++")
	void UpdateNickname() const;
};
