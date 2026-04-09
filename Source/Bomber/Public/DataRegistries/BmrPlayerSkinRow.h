// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "DalRegistryRow.h"
#include "Structures/BmrPlayerTag.h"

#include "BmrPlayerSkinRow.generated.h"

/**
 * Row struct for player skin materials registered via Data Registry.
 * Same PlayerTag can repeat: gameplay picks by index % count.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrPlayerSkinRow : public FTableRowBase
#if CPP
    , public TDalRegistryRow<FBmrPlayerSkinRow>
#endif
{
	GENERATED_BODY()

	/** The player character this skin belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** The material instance for this skin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UMaterialInstance> Material = nullptr;

	/** Gathers all skin row names for the given player tag from Data Registry */
	static void GetAllPlayerSkinNames(const FBmrPlayerTag& PlayerTag, TArray<FName>& OutSkinNames);

	/** Returns the total number of skin materials for the given player tag from Data Registry */
	static int32 GetSkinTexturesNum(const FBmrPlayerTag& PlayerTag);

	/** Returns the skin row name for the given player tag, picking by PlayerIndex % available skins count */
	static FName GetSkinRowName(const FBmrPlayerTag& PlayerTag, int32 PlayerIndex);

	/** Returns the skin material for the given player tag and skin index from Data Registry, or nullptr */
	static class UMaterialInterface* GetSkinMaterial(const FBmrPlayerTag& PlayerTag, int32 SkinIndex);
};
