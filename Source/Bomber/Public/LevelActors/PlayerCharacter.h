// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

/**
 * Numbers of power-ups that affect the abilities of a player during gameplay. 
 */
USTRUCT(BlueprintType)
struct FPowerUp
{
	GENERATED_BODY()

	/** Empty constructor */
	FPowerUp(){};

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
	 *		Public properties
	 * --------------------------------------------------- */
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++")
	class UMapComponent* MapComponent;  //[C.AW]

	/** All skeletal meshes of the character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TArray<class USkeletalMesh*> SkeletalMeshes;  //[M.DO]

	/** The static mesh nameplate */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UStaticMeshComponent* NameplateMeshComponent;  //[C.DO]

	/** All materials that used by nameplate meshes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "C++")
	TArray<class UMaterialInterface*> NameplateMaterials;  //[M.DO]

	/** The nickname of the character */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "C++")
	class UTextRenderComponent* NicknameTextRender;  //[C.DO]

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */

	/** Sets default values for this character's properties */
	APlayerCharacter();

	/** Called to bind functionality to input */
	virtual void
	SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */
	/** Count of items that affect the abilities of a player during gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	struct FPowerUp Powerups_;  //[AW]
	/** Items have access */
	friend class AItemActor;
	/** AI controller has access */
	friend class AMyAIController;

	/** The ID identification of the each character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected))
	int32 CharacterID_ = INDEX_NONE;  //[G]

	/* The AnimBlueprint class to use, can set it only in the gameplay */
	class TSubclassOf<UAnimInstance> MyAnimClass;

	/** The character's AI controller */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected))
	class AMyAIController* MyAIController;  //[G]

	/* ---------------------------------------------------
	 *		Protected functions
	 * --------------------------------------------------- */

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Spawns bomb on character position */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void SpawnBomb();
};
