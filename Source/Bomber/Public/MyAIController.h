// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Cell.h"

#include "MyAIController.generated.h"

/**
 * Characters controlled by bots
 */
UCLASS()
class BOMBER_API AMyAIController : public AAIController
{
	GENERATED_BODY()

public:
	/** @defgroup AI Functions of AI-characters */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	bool UpdateAI();

	/** Controlled character */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class AMyCharacter* MyCharacter;

protected:
	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	virtual void OnPossess(APawn* InPawn) final;

	/** Cell position of current path segment's end */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell AiMoveTo;
};
