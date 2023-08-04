// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"
//---
#include "Bomber.h" // ELevelType
#include "Misc/EnumRange.h"
//---
#include "FootTrailsTypes.generated.h"

/**
 * All the Foot Trails types.
 */
UENUM(BlueprintType)
enum class EFootTrailType : uint8
{
	None,
	Crossroad,
	DeadEnd,
	Straight,
	TJunction UMETA(DisplayName = "T-Junction"),
	Turn
};

ENUM_RANGE_BY_FIRST_AND_LAST(EFootTrailType, EFootTrailType::Crossroad, EFootTrailType::Turn);

/**
 * Specific foot trail archetype.
 */
USTRUCT(BlueprintType)
struct FOOTTRAILSGENERATORRUNTIME_API FFootTrailArchetype : public FTableRowBase
{
	GENERATED_BODY()

	/** The foot trail type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	EFootTrailType FootTrailType = EFootTrailType::Crossroad;

	/** The level type that this foot trail can be spawned on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	ELevelType LevelType = ELevelType::None;

	/** The foot trail mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TSoftObjectPtr<class UStaticMesh> Mesh = nullptr;

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FFootTrailArchetype& Other) const { return GetTypeHash(*this) == GetTypeHash(Other); }

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend FOOTTRAILSGENERATORRUNTIME_API uint32 GetTypeHash(const FFootTrailArchetype& Other) { return GetTypeHash(Other.FootTrailType) ^ GetTypeHash(Other.LevelType) ^ GetTypeHash(Other.Mesh); }
};
