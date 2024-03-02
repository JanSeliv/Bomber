// Copyright (c) Yevhenii Selivanov

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
//---
#include "EnvQueryTest_ExplosionCells.generated.h"

/**
 * Returns all cells in the explosion radius for a EQS test.
 */
UCLASS()
class NEWAI_API UEnvQueryTest_ExplosionCells : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_ExplosionCells();

protected:
	/** Starts the test. */
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	/** Returns proper description for the test. */
	virtual FText GetDescriptionDetails() const override;
};
