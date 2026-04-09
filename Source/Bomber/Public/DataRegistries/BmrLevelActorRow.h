// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DataTable.h"

// Bomber
#include "Bomber.h" // EBmrLevelType
#include "DalRegistryRow.h"

#include "BmrLevelActorRow.generated.h"

/**
 * Common base struct for all level actor Data Registry rows.
 * Provides LevelType and Mesh fields shared by Bomb, Box, Wall, Powerup, and Player rows.
 * UE5 struct inheritance is fully supported by Data Tables and Data Registries.
 */
USTRUCT(BlueprintType)
struct BOMBER_API FBmrLevelActorRow : public FTableRowBase
{
	GENERATED_BODY()

	/** The level where this actor visual should be used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBmrLevelType LevelType = ELT::None;

	/** The mesh of the level actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UStreamableRenderAsset> Mesh = nullptr;

	/** Returns cached level actor row by runtime struct type and row name, cast to base class.  */
	static const FBmrLevelActorRow* FindRowByName(const UScriptStruct* RowType, FName RowName);

	/** Finds first row matching the given level type with ELT::Max fallback, using runtime struct type.
	 * Polymorphic equivalent of TBmrLevelActorRow::GetRowByLevelType for runtime UScriptStruct* */
	static const FBmrLevelActorRow* FindRowByLevelType(const UScriptStruct* RowType, EBmrLevelType LevelType);

	/** Iterates all Data Registries whose ItemStruct inherits FBmrLevelActorRow */
	static void ForEachLevelActorRegistry(const TFunctionRef<void(class UDataRegistry* Registry, const UScriptStruct* ItemStruct)>& Callback);
};

/**
 * CRTP extension for level actor row structs that inherit FBmrLevelActorRow.
 * Adds LevelType + Mesh query methods using direct field access on the base struct.
 *
 * Usage:
 *   struct FBmrBombRow : public FBmrLevelActorRow, public TBmrLevelActorRow<FBmrBombRowData>
 *   { ... };
 *   const FBmrBombRow* Row = FBmrBombRow::GetRowByLevelType(ELT::Maya);
 */
template <typename TDerived>
struct TBmrLevelActorRow : public TDalRegistryRow<TDerived>
{
	/** Finds first row matching the given level type, falling back to ELT::Max (universal) */
	static FORCEINLINE const TDerived* GetRowByLevelType(EBmrLevelType LevelType)
	{
		return TDalRegistryRow<TDerived>::GetRowByPredicate([LevelType](const TDerived& Row)
		{
			return Row.LevelType == LevelType || Row.LevelType == ELT::Max;
		});
	}

	/** Returns DR row name for the given level type, falling back to ELT::Max, or NAME_None */
	static FORCEINLINE FName GetRowNameByLevelType(EBmrLevelType LevelType)
	{
		return TDalRegistryRow<TDerived>::GetRowNameByPredicate([LevelType](const TDerived& Row)
		{
			return Row.LevelType == LevelType || Row.LevelType == ELT::Max;
		});
	}
};
