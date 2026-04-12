// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataRegistries/BmrLevelActorRow.h"

// Bomber
#include "Structures/BmrPowerupTag.h"

#include "BmrPowerupRow.generated.h"

/**
 * Row struct for powerup data registered via Data Registry.
 * Mods or maps register their own data tables with powerup rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrPowerupRow : public FBmrLevelActorRow
#if CPP
    , public TDalRegistryRow<FBmrPowerupRow>
#endif
{
	GENERATED_BODY()

	/** The type of this powerup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBmrPowerupTag PowerupTag = FBmrPowerupTag::None;

	/** Gameplay effect to apply on collecting this powerup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UGameplayEffect> CollectGameplayEffect = nullptr;

	/** Gameplay effect to apply when the max amount of this powerup is reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<class UGameplayEffect> MaxCollectGameplayEffect = nullptr;

	/** Finds powerup row data by powerup tag from Data Registry */
	static const FBmrPowerupRow* GetRowByPowerupTag(FBmrPowerupTag Tag);
};
