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
	/** Controlled character */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "C++")
	class AMyCharacter* MyCharacter;

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "C++")
	bool UpdateAI(
		struct FCell& F0,		   ///< temporary param: the bot location
		TSet<struct FCell>& Free,  ///< temporary param: sides iterated cells
		bool& bIsDangerous);	   ///< temporary param: state meaning whether the bot is in an explosion

	/** Makes AI go toward specified destination cell */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MoveToCell(const struct FCell& DestinationCell);

protected:
	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) final;

	/** Allows the PlayerController to set up custom input bindings. */
	virtual void OnPossess(APawn* InPawn) final;

	/** Cell position of current path segment's end */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "C++")
	struct FCell AiMoveTo;
};
