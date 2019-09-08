// Copyright 2019 Yevhenii Selivanov.

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
	/** Sets default values for this character's properties */
	AMyAIController();

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateAI();

	/** Makes AI go toward specified destination cell */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MoveToCell(const struct FCell& DestinationCell);

protected:
	/** Cell position of current path segment's end */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties))
	struct FCell AiMoveTo;  //[G]

	/** Controlled character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected))
	class APlayerCharacter* MyCharacter;  //[G]

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Allows the PlayerController to set up custom input bindings. */
	virtual void OnPossess(APawn* InPawn) override;
};
