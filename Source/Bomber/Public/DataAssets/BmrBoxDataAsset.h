// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DataAssets/BmrLevelActorDataAsset.h"

#include "BmrBoxDataAsset.generated.h"

/**
 * Describes common data for all boxes.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrBoxDataAsset final : public UBmrLevelActorDataAsset
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UBmrBoxDataAsset();

	/** Returns the box data asset. */
	static const UBmrBoxDataAsset& Get();

	/** Returns default value from the data asset of the chance to spawn item after box destroying.
	 * It might be overridden by `Bomber.Box.SetPowerupsChance` cheat. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	int32 GetPowerupsChance() const;

protected:
	/** The chance to spawn item after box destroying. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties, ClampMin = "0", ClampMax = "100"))
	int32 SpawnItemChance = 30.f;
};