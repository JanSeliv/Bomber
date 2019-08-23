// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Cell.h"

#include "MyAIController.generated.h"

/**
 * Characters controlled by bots
 */
UCLASS()
class BOMBER_API AMyAIController final : public AAIController
{
	GENERATED_BODY()

public:
	/** Controlled character */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class AMyCharacter* MyCharacter;

	/** Sets default values for this character's properties */
	AMyAIController();

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	bool UpdateAI();

	/** Makes AI go toward specified destination cell */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MoveToCell(const struct FCell& DestinationCell);

protected:
	/** Cell position of current path segment's end */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell AiMoveTo;

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() final;

	/** Function called every frame on this AI controller to update movement */
	virtual void Tick(float DeltaTime) final;

	/** Allows the PlayerController to set up custom input bindings. */
	virtual void OnPossess(APawn* InPawn) final;
};
