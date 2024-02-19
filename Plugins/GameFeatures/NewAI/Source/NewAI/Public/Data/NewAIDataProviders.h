// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataProviders/AIDataProvider_QueryParams.h"
//---
#include "NewAIDataProviders.generated.h"

/**
 * Returns half of the larger grid size for 'Bind Data' property in EQS Query.
 */
UCLASS()
class NEWAI_API UNewAIDataProvider_HalfGridSize : public UAIDataProvider_QueryParams
{
	GENERATED_BODY()

public:
	virtual void BindData(const UObject& Owner, int32 RequestId) override;
};

/**
 * Returns the size of the cell in the grid for 'Bind Data' property in EQS Query.
 */
UCLASS()
class NEWAI_API UNewAIDataProvider_CellSize : public UAIDataProvider_QueryParams
{
	GENERATED_BODY()

public:
	virtual void BindData(const UObject& Owner, int32 RequestId) override;
};
