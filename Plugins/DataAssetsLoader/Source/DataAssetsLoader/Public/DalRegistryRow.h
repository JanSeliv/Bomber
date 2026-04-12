// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"

/**
 * Row-related helpers heavily used by TDalRegistryRow and utils
 */
struct DATAASSETSLOADER_API FDalRegistryRow
{
	/** Finds the Data Registry whose ItemStruct matches InStruct, cached for fast repeated access */
	static class UDataRegistry* GetRegistryForStruct(const UScriptStruct* InStruct);

	/** Returns overall number of cached rows for the given row struct type */
	static int32 GetRowsNum(const UScriptStruct* InStruct);

	/** Returns raw pointer to cached item data by struct type and RowName, O(1) lookup, or nullptr */
	static const uint8* GetRowByName(const UScriptStruct* InStruct, FName RowName);

	/** Returns raw pointer to cached item data at specified index for the given struct type, or nullptr */
	static const uint8* GetRowByIndex(const UScriptStruct* InStruct, int32 Index);

	/** Returns the first cached item for the given struct type, or nullptr */
	static FORCEINLINE const uint8* GetFirstRow(const UScriptStruct* InStruct) { return GetRowByIndex(InStruct, 0); }

	/** Returns the row name at specified index for the given struct type, or NAME_None */
	static FName GetRowNameByIndex(const UScriptStruct* InStruct, int32 Index);

	/** Returns the row name of the first cached item for the given struct type, or NAME_None */
	static FORCEINLINE FName GetFirstRowName(const UScriptStruct* InStruct) { return GetRowNameByIndex(InStruct, 0); }

	/** Iterates all cached items for the given struct type, calling Callback with raw item data */
	static void ForEachRow(const UScriptStruct* InStruct, const TFunctionRef<void(const uint8*)>& Callback);

	/** Iterates all cached items for the given struct type, calling Callback with item name and raw data */
	static void ForEachRowWithName(const UScriptStruct* InStruct, const TFunctionRef<void(FName ItemName, const uint8*)>& Callback);

	/** Finds first cached item matching predicate, or nullptr */
	static const uint8* GetRowByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate);

	/** Returns the row name of first cached item matching predicate, or NAME_None */
	static FName GetRowNameByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate);

	/** Counts cached items matching predicate */
	static int32 CountRowsByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate);

	/** Returns all Data Registries whose ItemStruct is a child of InBaseStruct */
	static void ForEachRegistry(const UScriptStruct* InBaseStruct, const TFunctionRef<void(class UDataRegistry* Registry, const UScriptStruct* ItemStruct)>& Callback);

	/** Casts raw registry item data to the expected row struct type */
	template <typename TRow>
	static FORCEINLINE const TRow* GetTypedRow(const uint8* Data) { return reinterpret_cast<const TRow*>(Data); }
};

/**
 * CRTP mixin that gives any FTableRowBase-derived struct self-contained Data Registry query API.
 * Consumers include only the row struct header, no Data Asset dependency.
 * UHT only parses the first base of USTRUCT, so this second C++ base is invisible to reflection.
 *
 * Usage:
 *   struct FMyRowData : public FTableRowBase, public TDalRegistryRow<FMyRowData>
 *   { ... };
 *   const FMyRowData* Row = FMyRowData::GetRowByName(TEXT("RowName"));
 */
template <typename TDerived>
struct TDalRegistryRow
{
	/** Returns the Data Registry associated with TDerived::StaticStruct() */
	static FORCEINLINE UDataRegistry* GetRegistry() { return FDalRegistryRow::GetRegistryForStruct(TDerived::StaticStruct()); }

	/** Returns overall number of cached rows */
	static FORCEINLINE int32 GetRowsNum() { return FDalRegistryRow::GetRowsNum(TDerived::StaticStruct()); }

	/** Returns the first cached row, or nullptr */
	static FORCEINLINE const TDerived* GetFirstRow() { return FDalRegistryRow::GetTypedRow<TDerived>(FDalRegistryRow::GetFirstRow(TDerived::StaticStruct())); }

	/** Returns the row name of the first cached row, or NAME_None */
	static FORCEINLINE FName GetFirstRowName() { return FDalRegistryRow::GetFirstRowName(TDerived::StaticStruct()); }

	/** Returns row data by DR row name, O(1) lookup, or nullptr */
	static FORCEINLINE const TDerived* GetRowByName(FName RowName) { return FDalRegistryRow::GetTypedRow<TDerived>(FDalRegistryRow::GetRowByName(TDerived::StaticStruct(), RowName)); }

	/** Finds first cached item matching predicate, or nullptr */
	static FORCEINLINE const TDerived* GetRowByPredicate(const TFunctionRef<bool(const TDerived&)>& Predicate)
	{
		return FDalRegistryRow::GetTypedRow<TDerived>(FDalRegistryRow::GetRowByPredicate(TDerived::StaticStruct(),
		    [&Predicate](const uint8* Data)
		{
			return Predicate(*FDalRegistryRow::GetTypedRow<TDerived>(Data));
		}));
	}

	/** Returns DR row name of first cached item matching predicate, or NAME_None */
	static FORCEINLINE FName GetRowNameByPredicate(const TFunctionRef<bool(const TDerived&)>& Predicate)
	{
		return FDalRegistryRow::GetRowNameByPredicate(TDerived::StaticStruct(),
		    [&Predicate](const uint8* Data)
		{
			return Predicate(*FDalRegistryRow::GetTypedRow<TDerived>(Data));
		});
	}

	/** Counts cached items matching predicate */
	static FORCEINLINE int32 CountRowsByPredicate(const TFunctionRef<bool(const TDerived&)>& Predicate)
	{
		return FDalRegistryRow::CountRowsByPredicate(TDerived::StaticStruct(),
		    [&Predicate](const uint8* Data)
		{
			return Predicate(*FDalRegistryRow::GetTypedRow<TDerived>(Data));
		});
	}

	/** Gathers all cached items matching predicate */
	static FORCEINLINE void GetRowsByPredicate(TArray<const TDerived*>& OutRows, const TFunctionRef<bool(const TDerived&)>& Predicate)
	{
		FDalRegistryRow::ForEachRow(TDerived::StaticStruct(), [&OutRows, &Predicate](const uint8* Data)
		{
			const TDerived* Row = FDalRegistryRow::GetTypedRow<TDerived>(Data);
			if (Predicate(*Row))
			{
				OutRows.Emplace(Row);
			}
		});
	}

	/** Iterates all cached rows */
	static FORCEINLINE void ForEachRow(const TFunctionRef<void(const TDerived&)>& Callback)
	{
		FDalRegistryRow::ForEachRow(TDerived::StaticStruct(), [&Callback](const uint8* Data)
		{
			Callback(*FDalRegistryRow::GetTypedRow<TDerived>(Data));
		});
	}

	/** Iterates all cached rows with their row names */
	static FORCEINLINE void ForEachRowWithName(const TFunctionRef<void(FName RowName, const TDerived&)>& Callback)
	{
		FDalRegistryRow::ForEachRowWithName(TDerived::StaticStruct(), [&Callback](FName RowName, const uint8* Data)
		{
			Callback(RowName, *FDalRegistryRow::GetTypedRow<TDerived>(Data));
		});
	}
};
