// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataRegistries/BmrLevelActorRow.h"

#include "BmrBoxRow.generated.h"

/**
 * Row struct for box data registered via Data Registry.
 * Mods or maps register their own data tables with box rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrBoxRow : public FBmrLevelActorRow
#if CPP
    , public TBmrLevelActorRow<FBmrBoxRow>
#endif
{
	GENERATED_BODY()
};
