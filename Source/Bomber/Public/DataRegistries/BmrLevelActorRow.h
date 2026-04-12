// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "DalRegistryRow.h"

#include "BmrLevelActorRow.generated.h"

/**
 * Common base struct for all level actor Data Registry rows.
 * Provides Mesh field and helpers shared by Bomb, Box, Wall, Powerup, and Player rows.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrLevelActorRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The mesh of the level actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UStreamableRenderAsset> Mesh = nullptr;

	/** Returns cached level actor row by runtime struct type and row name, cast to base class */
	static const FBmrLevelActorRow* FindRowByName(const UScriptStruct* RowType, FName RowName);

	/** Returns the first cached level actor row for the given runtime struct type, or nullptr */
	static const FBmrLevelActorRow* FindFirstRow(const UScriptStruct* RowType);
};
