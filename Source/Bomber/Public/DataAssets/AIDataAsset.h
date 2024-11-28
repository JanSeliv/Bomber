// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/MyPrimaryDataAsset.h"
//---
#include "AIDataAsset.generated.h"

/**
* Contains AI data.
*/
UCLASS(Blueprintable, BlueprintType)
class BOMBER_API UAIDataAsset final : public UMyPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the AI data asset. */
	static const UAIDataAsset& Get();

	/** Returns the search radius of items.
	  * @see UAIDataAsset::ItemSearchRadiusInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetItemSearchRadius() const { return ItemSearchRadiusInternal; }

	/** Returns the search radius of crossways
	  * @see UAIDataAsset::CrosswaySearchRadiusInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetCrosswaySearchRadius() const { return CrosswaySearchRadiusInternal; }

	/** Returns the filter radius of near cells.
	  * @see UAIDataAsset::NearFilterRadiusInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNearFilterRadius() const { return NearFilterRadiusInternal; }

	/** Returns the radius of dangerous cells.
	* @see UAIDataAsset::DangerousCellRadiusInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetNearDangerousRadius() const { return NearDangerousRadiusInternal; }

protected:
	/** The search radius of items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Item Search Radius", ShowOnlyInnerProperties))
	int32 ItemSearchRadiusInternal = 2;

	/** The search radius of crossways. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Crossway Search Radius", ShowOnlyInnerProperties))
	int32 CrosswaySearchRadiusInternal = 2;

	/** Determine radius of near dangerous cells (length <= near dangerous radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Near Dangerous Radius", ShowOnlyInnerProperties))
	int32 NearDangerousRadiusInternal = 3;

	/** Determine filter radius of near cells (length <= near radius). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BlueprintProtected, DisplayName = "Near Filter Radius", ShowOnlyInnerProperties))
	int32 NearFilterRadiusInternal = 3;
};