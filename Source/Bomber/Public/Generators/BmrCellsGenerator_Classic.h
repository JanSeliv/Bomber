// Copyright (c) Yevhenii Selivanov

#pragma once

#include "BmrCellsGenerator_Base.h"

#include "BmrCellsGenerator_Classic.generated.h"

/**
 * Generates a level using the classic layout.
 * It places indestructible walls on all uneven grid coordinates (e.g., 1x1, 1x3, 3x1)
 * and fills the remaining space with destructible boxes, leaving safe zones around the player.
 * Example Layout (9x9 with ~50% Box Chance):
 * P . □ . . . . . P
 * . █ . █ □ █ □ █ .
 * □ . █ . █ . █ . □
 * . █ □ █ . █ . █ □
 * █ . █ . █ . █ . █
 * □ █ . █ □ █ . █ .
 * . . █ . █ . █ . □
 * . █ □ █ . █ □ █ .
 * P . . □ . . □ . P
 * (P=Player, █=Wall, □=Box)
 */
UCLASS(DisplayName = "Classic")
class BOMBER_API UBmrCellsGenerator_Classic : public UBmrCellsGenerator_Base
{
	GENERATED_BODY()

public:
	/** Is overriden to implement custom level generation logic. */
	virtual TMap<FBmrCell, EBmrActorType> GenerateLevel(FBmrGeneratorData&& GeneratorData) override;
};