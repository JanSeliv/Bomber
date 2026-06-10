// Copyright (c) Yevhenii Selivanov

#pragma once

#include "DalPrimaryDataAsset.h"

// Bomber
#include "Structures/BmrGameDifficultyTag.h"

// UE
#include "GameplayTagContainer.h"

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

	/** Returns difficulties where AI ignores powerups. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetIgnorePowerups() const { return IgnorePowerups; }

	/** Returns difficulties where AI ignores players. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetIgnorePlayers() const { return IgnorePlayers; }

	/** Returns difficulties where AI ignores traps, wandering into corners. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetIgnoreTraps() const { return IgnoreTraps; }

	/** Returns difficulties where AI never places bombs. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetIgnoreBombPlacement() const { return IgnoreBombPlacement; }

	/** Returns difficulties where AI ignores danger, stepping into explosions. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	const FGameplayTagContainer& GetIgnoreDanger() const { return IgnoreDanger; }

	/** Returns decision-delay multiplier for difficulty, 1 keeps full reaction when unlisted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	float GetReactionSlowdown(FBmrGameDifficultyTag DifficultyTag) const;

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

	/** Difficulties where AI ignores powerups. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "Difficulty"))
	FGameplayTagContainer IgnorePowerups = FGameplayTagContainer::EmptyContainer;

	/** Difficulties where AI ignores players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "Difficulty"))
	FGameplayTagContainer IgnorePlayers = FGameplayTagContainer::EmptyContainer;

	/** Difficulties where AI ignores traps, wandering into corners. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "Difficulty"))
	FGameplayTagContainer IgnoreTraps = FGameplayTagContainer::EmptyContainer;

	/** Difficulties where AI never places bombs. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "Difficulty"))
	FGameplayTagContainer IgnoreBombPlacement = FGameplayTagContainer::EmptyContainer;

	/** Difficulties where AI ignores danger, stepping into explosions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected, ShowOnlyInnerProperties, Categories = "Difficulty"))
	FGameplayTagContainer IgnoreDanger = FGameplayTagContainer::EmptyContainer;

	/** Difficulties where AI reacts slower, value multiplies decision delay. Unlisted react at full reaction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty", meta = (BlueprintProtected))
	TMap<FBmrGameDifficultyTag, float /*DelayMultiplier*/> ReactionSlowdown;
};