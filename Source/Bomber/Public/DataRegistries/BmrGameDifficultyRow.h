// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "DalRegistryRow.h"
#include "Structures/BmrGameDifficultyTag.h"

#include "BmrGameDifficultyRow.generated.h"

/**
 * Row struct for difficulty data table registered via Data Registry.
 * Mods register their own data tables with custom difficulty rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrGameDifficultyRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrGameDifficultyRow>
#endif
{
	GENERATED_BODY()

	/** The gameplay tag representing this difficulty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrGameDifficultyTag DifficultyTag = FBmrGameDifficultyTag::None;

	/** Integer for settings combobox index and curve table evaluation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DifficultyLevel = INDEX_NONE;

	/** Localized display name shown in settings combobox. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName = FText::GetEmpty();
};
