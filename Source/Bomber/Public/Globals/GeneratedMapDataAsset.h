// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Globals/LevelActorDataAsset.h"
//---
#include "GeneratedMapDataAsset.generated.h"

/**
 * Unique data about one separated level.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FLevelStreamRow
{
	GENERATED_BODY()

	/** The empty mesh row. */
	static const FLevelStreamRow Empty;

	/** Default constructor */
	FLevelStreamRow() = default;

	/** The level asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<class UWorld> Level = nullptr;

	/** The associated type of a level. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	ELevelType LevelType = ELT::None;

	/** The name of a level on UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	FText LevelName = TEXT_NONE;
};

/**
 * Contains all data that describe all levels.
 */
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UGeneratedMapDataAsset final : public UBomberDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the generated map data asset. */
	static const UGeneratedMapDataAsset& Get();

	/** Returns level rows. */
	UFUNCTION(BlueprintPure, Category = "C++")
	void GetLevelStreamRows(TArray<FLevelStreamRow>& OutRows) const { OutRows = LevelsInternal; }

	/** Get UGeneratedMapDataAsset::WallsChanceInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetWallsChance() const { return WallsChanceInternal; }

	/** Get UGeneratedMapDataAsset::BoxesChanceInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetBoxesChance() const { return BoxesChanceInternal; }

	/** Get UGeneratedMapDataAsset::CollisionsAssetInternal. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE TSubclassOf<class AActor> GetCollisionsAssetClass() const { return CollisionsAssetInternal; }

	/** Get UGeneratedMapDataAsset::LockLocationOnZeroInternal.  */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsLockedOnZero() const { return LockOnZeroInternal; }

protected:
	/** Contains all used levels. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Levels", TitleProperty = "LevelType", ShowOnlyInnerProperties))
	TArray<FLevelStreamRow> LevelsInternal;

	/** The chance of walls generation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Walls Chance", ShowOnlyInnerProperties, Units = "Percent", ClampMin = "0", ClampMax = "100"))
	int32 WallsChanceInternal = 35;

	/** The chance of boxes generation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Boxes Chance", ShowOnlyInnerProperties, Units = "Percent", ClampMin = "0", ClampMax = "100"))
	int32 BoxesChanceInternal = 70;

	/** Asset that contains scalable collision. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Collisions Asset", ShowOnlyInnerProperties))
	TSubclassOf<class AActor> CollisionsAssetInternal = nullptr;

	/** If true, the level position will be locked on the (0,0,0) location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Lock Location On Zero", ShowOnlyInnerProperties))
	bool LockOnZeroInternal = true;
};