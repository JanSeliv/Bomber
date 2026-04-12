// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrPlayerTag.h"

#include "BmrAIDataAsset.generated.h"

/**
 * Contains AI data.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UBmrAIDataAsset final : public UDalPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the AI data asset. */
	static const UBmrAIDataAsset& Get();

	/** Returns the search radius of powerups.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetPowerupSearchRadius() const { return PowerupSearchRadius; }

	/** Returns the search radius of crossways.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetCrosswaySearchRadius() const { return CrosswaySearchRadius; }

	/** Returns the filter radius of near cells.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetNearFilterRadius() const { return NearFilterRadius; }

	/** Returns the radius of dangerous cells.*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetNearDangerousRadius() const { return NearDangerousRadius; }

	/** Returns the player tag used as default character for all AI players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FORCEINLINE FBmrPlayerTag& GetDefaultAIPlayerTag() const { return DefaultAIPlayerTag; }

protected:
	/** The search radius of powerups. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 PowerupSearchRadius = 2;

	/** The search radius of crossways. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 CrosswaySearchRadius = 2;

	/** Determine radius of near dangerous cells (length <= near dangerous radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 NearDangerousRadius = 3;

	/** Determine filter radius of near cells (length <= near radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	int32 NearFilterRadius = 3;

	/** The player tag used as default character for all AI players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, ShowOnlyInnerProperties))
	FBmrPlayerTag DefaultAIPlayerTag = FBmrPlayerTag::None;
};