// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"
//---
#include "Bomber.h" // ELevelType
#include "Misc/EnumRange.h"
//---
#include "FTGTypes.generated.h"

/**
 * All the Foot Trails types.
 */
UENUM(BlueprintType)
enum class EFTGTrailType : uint8
{
	None,
	Crossroad,
	DeadEnd,
	Straight,
	TJunction UMETA(DisplayName = "T-Junction"),
	Turn
};

ENUM_RANGE_BY_FIRST_AND_LAST(EFTGTrailType, EFTGTrailType::Crossroad, EFTGTrailType::Turn);

/**
 * Specific foot trail archetype.
 */
USTRUCT(BlueprintType)
struct FOOTTRAILSGENERATORRUNTIME_API FFTGArchetype : public FTableRowBase
{
	GENERATED_BODY()

	/** The foot trail type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	EFTGTrailType FootTrailType = EFTGTrailType::Crossroad;

	/** The level type that this foot trail can be spawned on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	ELevelType LevelType = ELevelType::None;

	/** The foot trail mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	TSoftObjectPtr<class UStaticMesh> Mesh = nullptr;

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FFTGArchetype& Other) const { return GetTypeHash(*this) == GetTypeHash(Other); }

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend FOOTTRAILSGENERATORRUNTIME_API uint32 GetTypeHash(const FFTGArchetype& Other) { return GetTypeHash(Other.FootTrailType) ^ GetTypeHash(Other.LevelType) ^ GetTypeHash(Other.Mesh); }
};
