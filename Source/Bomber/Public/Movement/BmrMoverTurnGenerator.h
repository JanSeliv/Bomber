// Copyright (c) Yevhenii Selivanov

#pragma once

#include "MoveLibrary/ModularMovement.h"

#include "BmrMoverTurnGenerator.generated.h"

/**
 * Extends the Linear Turn Generator to add functionality for orienting rotation to movement direction.
 */
UCLASS()
class BOMBER_API UBmrMoverTurnGenerator : public ULinearTurnGenerator
{
	GENERATED_BODY()

public:
	/** Whether to orient rotation to movement direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	bool bOrientRotationToMovement = true;

	/** Is overridden to handle orientation-related movement. */
	virtual FVector GetTurn_Implementation(FRotator TargetOrientation, const FMoverTickStartData& FullStartState, const struct FMoverDefaultSyncState& MoverState, const FMoverTimeStep& TimeStep, const FProposedMove& ProposedMove, UMoverBlackboard* SimBlackboard) override;
};
