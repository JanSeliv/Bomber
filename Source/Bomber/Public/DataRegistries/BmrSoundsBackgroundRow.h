// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "DalRegistryRow.h"

#include "BmrSoundsBackgroundRow.generated.h"

/**
 * Row struct for background music data registered via Data Registry.
 * Mods or maps register their own data tables with background music rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrSoundsBackgroundRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrSoundsBackgroundRow>
#endif
{
	GENERATED_BODY()

	/** The background music sound asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class USoundBase> Music = nullptr;
};
