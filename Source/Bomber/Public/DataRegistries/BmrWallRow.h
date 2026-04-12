// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataRegistries/BmrLevelActorRow.h"

#include "BmrWallRow.generated.h"

/**
 * Row struct for wall data registered via Data Registry.
 * Mods or maps register their own data tables with wall rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrWallRow : public FBmrLevelActorRow
#if CPP
    , public TDalRegistryRow<FBmrWallRow>
#endif
{
	GENERATED_BODY()
};
