// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"

#include "MyCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPowerUp
{
	GENERATED_BODY()

	FPowerUp(){};

	// Increase the movement speed of the character
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 SkateN = 1;

	// Increase the number of bombs that can be set at one time
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 BombN = 1;

	//  Increase the bomb blast radius
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int32 FireN = 1;
};

UCLASS()
class BOMBER_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) final;

	/** The MapComponent manages this actor on the Level Map */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "C++")
	class UMapComponent* MapComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	/** Called after an actor has been moved in the editor */
	virtual void PostEditMove(bool bFinished) override;
#endif  //WITH_EDITOR [Editor]

	/** Called when this actor is explicitly being destroyed */
	virtual void Destroyed() override;

	/** Spawn bomb on character position */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SpawnBomb();

	// Count of items that affect the abilities of a player during gameplay
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	struct FPowerUp Powerups_;
	friend class AItem;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	int32 CharacterID_ = -1;
};
