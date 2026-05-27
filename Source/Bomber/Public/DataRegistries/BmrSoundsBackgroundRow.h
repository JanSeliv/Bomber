// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h" // FGameplayTagQuery

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

	/** Gameplay tag listened on world ASC, music plays while requirements are met, removed otherwise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery ActivityRequirements = FGameplayTagQuery::EmptyQuery;

	/** The background music sound asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class USoundBase> Music = nullptr;
};
