// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPowerUp
{
	GENERATED_BODY()

public:
	FPowerUp(){};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector location;

	// Increase the movement speed of the character
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int skateNo = 1;

	// Increase the number of bombs that can be set at one time
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int bombNo = 1;

	//  Increase the bomb blast radius
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	int fireNo = 1;
};

UCLASS()
class BOMBER_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly, Category = "C++")
	class UMapComponent* mapComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Count of items that affect the abilities of a player during gameplay
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	FPowerUp powerups;
};
