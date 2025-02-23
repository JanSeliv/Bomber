// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CustomPlayerMeshData.generated.h"

/**
 * Is runtime representation of the read-only Player Row.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Bomber.PlayerMeshDataUtils.MakeCustomPlayerMeshData"))
struct BOMBER_API FCustomPlayerMeshData
{
	GENERATED_BODY()

	/** Empty data. */
	static const FCustomPlayerMeshData Empty;

	/** Default constructor. */
	FCustomPlayerMeshData() = default;

	/** Constructor that initializes the player data by specified tag. */
	FCustomPlayerMeshData(const struct FPlayerTag& PlayerTag, int32 InSkinIndex);

	/** Constructor that initializes the data directly. */
	FCustomPlayerMeshData(const class UPlayerRow& InPlayerRow, int32 InSkinIndex);

	/** The row that is used to visualize the bomber character. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	TObjectPtr<const class UPlayerRow> PlayerRow = nullptr;

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return PlayerRow != nullptr; }

	/** Equality operator to compare the mesh data. */
	FORCEINLINE bool operator==(const FCustomPlayerMeshData& Other) const { return PlayerRow == Other.PlayerRow && SkinIndex == Other.SkinIndex; }

	/*********************************************************************************************
	 * Skins
	 ********************************************************************************************* */
public:
	/** The index of the texture is currently set, since this data represents the row, where multiple skins can be stored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 SkinIndex = 0;

	/** Bitmask for available skins (up to 32 skins).
	 * Each bit represents a skin: 0 = locked, 1 = unlocked.
	 * By default, all skins are unlocked.
	 * 0001 -> Only first skin is unlocked
	 * 0111 -> First three skins are unlocked
	 * 1111 -> All skins are unlocked */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++")
	int32 SkinAvailabilityMask = TNumericLimits<int32>::Max();
};