// Copyright (c) Yevhenii Selivanov

#pragma once

#include "BmrMeshData.generated.h"

/**
 * Is runtime representation of the read-only Level Actor Row (Player, Bomb etc)
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrMeshData
{
	GENERATED_BODY()

	/** Empty data. */
	static const FBmrMeshData Empty;

	/** Default constructor. */
	FBmrMeshData() = default;

	/** Constructor that initializes the data directly. */
	FBmrMeshData(const class UBmrLevelActorRow* InRow, int32 InSkinIndex = 0);

	/** The row that is used to visualize the bomber character. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "[Bomber]")
	TObjectPtr<const class UBmrLevelActorRow> Row = nullptr;

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return Row != nullptr; }

	/** Equality operator to compare the mesh data. */
	FORCEINLINE bool operator==(const FBmrMeshData& Other) const { return Row == Other.Row && SkinIndex == Other.SkinIndex; }

	/*********************************************************************************************
	 * Skins
	 ********************************************************************************************* */
public:
	/** The index of the texture is currently set, since this data represents the row, where multiple skins can be stored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "[Bomber]")
	int32 SkinIndex = 0;

	/** Bitmask for available skins (up to 32 skins).
	 * Each bit represents a skin: 0 = locked, 1 = unlocked.
	 * By default, all skins are unlocked.
	 * 0001 -> Only first skin is unlocked
	 * 0111 -> First three skins are unlocked
	 * 1111 -> All skins are unlocked */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "[Bomber]")
	int32 SkinAvailabilityMask = TNumericLimits<int32>::Max();
};