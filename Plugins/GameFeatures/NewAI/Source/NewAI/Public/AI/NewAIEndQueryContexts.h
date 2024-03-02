// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "NewAIEndQueryContexts.generated.h"

/**
 * Returns level center as a context for EQS Query.
 */
UCLASS()
class NEWAI_API UNewAIEndQueryContext_Level : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * Returns all cells on level for EQS Query.
 */
UCLASS()
class NEWAI_API UNewAIEndQueryContext_AllCellsOnLevel : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * Returns player as a context for EQS Query.
 */
UCLASS()
class NEWAI_API UNewAIEndQueryContext_Player : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
