// Copyright (c) Yevhenii Selivanov

#pragma once

#include "BmrCellsGenerator_Base.h"

#include "BmrCellsGenerator_FourSidedSymmetry.generated.h"

/**
 * Contains all input data required to generate the core path network for a quarter of the map.
 */
struct FBmrGeneratorQuarterPathParams
{
	const FBmrCells& QuarterCells;
	const FBmrCell& StartCell;
	const FIntPoint& QuarterScale;
	const FBmrCells& DraggedWalls;
	const FBmrCells& PriorityTargets;
};

/**
 * Contains all input data required for the obstacle-filling phase of generation.
 */
struct FBmrGeneratorQuarterFillParams
{
	FBmrCells&& PathCells;
	const FBmrCell& PlayerStartCell;
};

/**
 * Wraps all parameters for the FindDirectedPathWithDetours function.
 */
struct FBmrGeneratorPathfindParams
{
	const FBmrCell& StartNode;
	const FBmrCells& TargetCells;
	const FIntPoint& PrimaryDirection;
	const TArray<FIntPoint>& SecondaryDirections;
};

/**
 * Creates a random but fair map by generating one-quarter of the level
 * and then mirroring it to create a perfectly symmetrical battlefield.
 * It guarantees that paths exist between all players, boxes and to any dragged powerups.
 * Example Layout (P=Player, █=Wall, □=Box):
 * P . □ █ █ □ . P
 * . . □ . . □ . .
 * □ □ . . . . □ □
 * █ . . . . . . █
 * █ . . . . . . █
 * □ □ . . . . □ □
 * . . □ . . □ . .
 * P . □ █ █ □ . P
 */
UCLASS(DisplayName = "4-Sided Symmetry")
class BOMBER_API UBmrCellsGenerator_FourSidedSymmetry : public UBmrCellsGenerator_Base
{
	GENERATED_BODY()

protected:
	/** If enabled, displays the calculated core path for connecting dragged powerups. */
	UPROPERTY(EditDefaultsOnly)
	bool bDisplayPrimaryPath = false;

	/** If enabled, displays the calculated path connecting the network to the right border. */
	UPROPERTY(EditDefaultsOnly)
	bool bDisplayRightPath = false;

	/** If enabled, displays the calculated path connecting the network to the bottom border. */
	UPROPERTY(EditDefaultsOnly)
	bool bDisplayBottomPath = false;

public:
	/** Is overriden to implement custom level generation logic. */
	virtual TMap<FBmrCell, EBmrActorType> GenerateLevel(FBmrGeneratorData&& GeneratorData) override;

private:
	// --- Helper Methods ---

	/** Creates a network of paths from the player start to priority targets and quarter borders. */
	FBmrCells GeneratePathsForQuarter(const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData);

	/** Fills the quarter with walls and boxes based on fill percentages. */
	static TMap<FBmrCell, EBmrActorType> FillObstaclesInQuarter(FBmrGeneratorQuarterFillParams&& FillParams, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData);

	/** Applies four-sided symmetry to the generated quarter map to build the full level. */
	static TMap<FBmrCell, EBmrActorType> ApplySymmetry(TMap<FBmrCell, EBmrActorType>&& QuarterActors, const FBmrGeneratorData& GeneratorData);

	/** Creates a directed but randomized path. */
	static FBmrCellsArr FindDirectedPathWithDetours(const FBmrGeneratorPathfindParams& Params, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData);

	/** Finds all reachable cells from a starting point. */
	static FBmrCells FindWalkableArea(const FBmrCell& StartCell, const FBmrCells& WallCells, const FBmrGeneratorQuarterPathParams& PathParams, const FBmrGeneratorData& GeneratorData);
};