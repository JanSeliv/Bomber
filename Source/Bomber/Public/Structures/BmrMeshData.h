// Copyright (c) Yevhenii Selivanov

#pragma once

#include "BmrMeshData.generated.h"

/**
 * Is runtime representation of a Data Registry row for a level actor (Player, Bomb etc).
 * All visual data (mesh, player tag, etc) is resolved from the RowName via DR lookup
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrMeshData
{
	GENERATED_BODY()

	/** Empty data. */
	static const FBmrMeshData Empty;

	/** Default constructor. */
	FBmrMeshData() = default;

	/** The Data Registry row name that identifies this level actor's visual data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "[Bomber]")
	FName RowName = NAME_None;

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return !RowName.IsNone(); }

	/** Equality operator to compare the mesh data. */
	FORCEINLINE bool operator==(const FBmrMeshData& Other) const { return RowName == Other.RowName && SkinRowName == Other.SkinRowName; }

	/*********************************************************************************************
	 * Skins
	 ********************************************************************************************* */
public:
	/** The FBmrPlayerSkinRow row name of the currently applied skin, stable across Data Registry mutations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	FName SkinRowName = NAME_None;

	/** Bitmask value where every skin bit is set, meaning all skins are unlocked. */
	static constexpr int32 AllSkinsAvailableMask = TNumericLimits<int32>::Max();

	/** Bitmask for available skins (up to 32 skins).
	 * Each bit represents a skin: 0 = locked, 1 = unlocked.
	 * By default, all skins are unlocked.
	 * 0001 -> Only first skin is unlocked
	 * 0111 -> First three skins are unlocked
	 * 1111 -> All skins are unlocked */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "[Bomber]")
	int32 SkinAvailabilityMask = AllSkinsAvailableMask;
};