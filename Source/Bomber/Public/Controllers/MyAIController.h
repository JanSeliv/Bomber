// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "AIController.h"
//---
#include "Structures/Cell.h"
//---
#include "MyAIController.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Characters controlled by bots.
* @see Access its data with UAIDataAsset (Content/Bomber/DataAssets/DA_AI).
 */
UCLASS()
class BOMBER_API AMyAIController final : public AAIController
{
	GENERATED_BODY()

public:
	/* ---------------------------------------------------
	*		Public functions
	* --------------------------------------------------- */

	/** Sets default values for this character's properties */
	AMyAIController();

	/** Makes AI go toward specified destination cell */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void MoveToCell(const FCell& DestinationCell);

protected:
	/* ---------------------------------------------------
	*		Protected properties
	* --------------------------------------------------- */

	/** Timer to update AI. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "AI Update Handle"))
	FTimerHandle AIUpdateHandleInternal;

	/** Cell position of current path segment's end */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, ShowOnlyInnerProperties, DisplayName = "AI Move To"))
	FCell AIMoveToInternal = FCell::InvalidCell;

	/** Controlled character */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Owner Character"))
	TObjectPtr<class APlayerCharacter> OwnerInternal = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */

	/** Called when an instance of this class is placed (in editor) or spawned */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** This is called only in the gameplay before calling begin play. */
	virtual void PostInitializeComponents() override;

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Allows the controller to react on possessing the pawn to enable AI. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Allows the controller to react on unpossessing the pawn to disable AI. */
	virtual void OnUnPossess() override;

	/** Locks or unlocks movement input. */
	virtual void SetIgnoreMoveInput(bool bShouldIgnore) override;

	/** Stops running to target. */
	virtual void Reset() override;

	/** The main AI logic */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void UpdateAI();

	/** Enable or disable AI for this bot. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (BlueprintProtected))
	void SetAI(bool bShouldEnable);

	/** Listen game states to enable or disable AI. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);
};
