// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataRegistries/BmrLevelActorRow.h"

// Bomber
#include "Structures/BmrPlayerTag.h"

#include "BmrPlayerRow.generated.h"

/**
 * Row struct for player character data registered via Data Registry.
 * Mods or maps can register their own data tables with player rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrPlayerRow : public FBmrLevelActorRow
#if CPP
    , public TBmrLevelActorRow<FBmrPlayerRow>
#endif
{
	GENERATED_BODY()

	/** The tag of this player character for association with other data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPlayerTag PlayerTag = FBmrPlayerTag::None;

	/** Gameplay effect to apply on changing to this character. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UGameplayEffect> ConfigGameplayEffect = nullptr;

	/** The movement animation blend space for this character. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UBlendSpace1D> IdleWalkRunBlendSpace = nullptr;

	/** Dance animation used mostly in the menu instead of idle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UAnimSequence> DanceAnimation = nullptr;

	/** Death animation montage played on character death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UAnimMontage> DeathMontage = nullptr;

	/** Returns player row data by player tag from Data Registry, or nullptr */
	static const FBmrPlayerRow* GetRowByPlayerTag(const FBmrPlayerTag& PlayerTag);

	/** Returns the Data Registry row name for the given player tag, or NAME_None */
	static FName GetRowNameByPlayerTag(const FBmrPlayerTag& PlayerTag);

	/** Returns player row data by mesh from Data Registry, returns empty row data if not found */
	static const FBmrPlayerRow& GetRowByMesh(const class UStreamableRenderAsset* Mesh);
};
