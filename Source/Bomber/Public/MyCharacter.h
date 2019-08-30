// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"

#include "MyCharacter.generated.h"

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
class BOMBER_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

	/** All skeletal meshes of the character */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "C++")
	TArray<class USkeletalMesh*> SkeletalMeshes;

	/** The static mesh nameplate */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "C++")
	class UStaticMeshComponent* NameplateMeshComponent;

	/** All materials that used by nameplate meshes */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "C++")
	TArray<class UMaterialInterface*> NameplateMaterials;

	/** The nickname of the character */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "C++")
	class UTextRenderComponent* NicknameTextRender;

	/* ---------------------------------------------------
	 *	MyCharacter's public functions
	 * --------------------------------------------------- */

	/** Sets default values for this character's properties */
	AMyCharacter();

	/** Called to bind functionality to input */
	virtual void
	SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) final;

protected:
	/** Count of items that affect the abilities of a player during gameplay */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	struct FPowerUp Powerups_;
	/** Items have access */
	friend class AItemActor;
	/** AI controller has access */
	friend class AMyAIController;

	/** The ID identification of each character */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	int32 CharacterID_ = INDEX_NONE;

	/* The AnimBlueprint class to use, can set it only in gameplay */
	class TSubclassOf<UAnimInstance> MyAnimClass;

	/** The character's AI controller */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	class AMyAIController* MyAIController;

	/* ---------------------------------------------------
	 *	MyCharacter's protected functions
	 * --------------------------------------------------- */

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** Spawns bomb on character position */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SpawnBomb();
};
