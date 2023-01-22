// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/LevelActorDataAsset.h"
//---
#include "BoxDataAsset.generated.h"

/**
 * Describes common data for all boxes.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBoxDataAsset final : public ULevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBoxDataAsset();

	/** Returns the box data asset. */
	static const UBoxDataAsset& Get();

	/** Returns the chance to spawn item after box destroying. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetSpawnItemChance() const { return SpawnItemChanceInternal; }

protected:
	/** The chance to spawn item after box destroying. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Spawn Item Chance", ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 SpawnItemChanceInternal = 30.F;
};