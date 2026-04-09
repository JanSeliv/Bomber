// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "DalRegistryRow.h"
#include "Structures/BmrPlayerTag.h"

#include "BmrPlayerPropRow.generated.h"

/**
 * Row struct for player prop attachments registered via Data Registry.
 * Same PlayerTag can repeat: gameplay picks by index % count.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrPlayerPropRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrPlayerPropRow>
#endif
{
	GENERATED_BODY()

	/** The player character this prop belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** The attached static or skeletal mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UStreamableRenderAsset> Mesh = nullptr;

	/** The socket where this prop should be attached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Socket = NAME_None;

	/** Is optional loop animation played on the prop after attaching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UAnimSequence> MeshAnimation = nullptr;

	/** Gathers all player prop rows for the given player tag from Data Registry */
	static void GetPlayerProps(const FBmrPlayerTag& PlayerTag, TArray<const FBmrPlayerPropRow*>& OutProps);
};
