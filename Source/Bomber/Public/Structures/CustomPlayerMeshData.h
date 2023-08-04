// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CustomPlayerMeshData.generated.h"

/**
 * Determines how the character looks.
 * Contains additional data.
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

	/** The index of the texture to set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++")
	int32 SkinIndex = 0;

	/** Returns true is data is valid. */
	FORCEINLINE bool IsValid() const { return PlayerRow != nullptr; }
};
