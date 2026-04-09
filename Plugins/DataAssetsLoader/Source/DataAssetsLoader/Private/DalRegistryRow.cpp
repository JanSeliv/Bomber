// Copyright (c) Yevhenii Selivanov

#include "DalRegistryRow.h"

// UE
#include "DataRegistry.h"
#include "DataRegistrySubsystem.h"

namespace DalRegistryRowInternal
{
	/** Shared implementation: gets all cached items from a registry */
	static bool GetAllCachedItems(const UDataRegistry* Registry, TMap<FDataRegistryId, const uint8*>& OutItems)
	{
		if (!Registry)
		{
			return false;
		}
		const UScriptStruct* ItemStruct = nullptr;
		Registry->GetAllCachedItems(OutItems, ItemStruct);
		return true;
	}
} // namespace DalRegistryRowInternal

// Finds the Data Registry whose ItemStruct matches InStruct, cached for fast repeated access
UDataRegistry* FDalRegistryRow::GetRegistryForStruct(const UScriptStruct* InStruct)
{
	if (!InStruct)
	{
		return nullptr;
	}

	// Check cache first
	static TMap<const UScriptStruct*, TWeakObjectPtr<UDataRegistry>> StructToRegistryCache;
	if (const TWeakObjectPtr<UDataRegistry>* Found = StructToRegistryCache.Find(InStruct))
	{
		if (UDataRegistry* CachedRegistry = Found->Get())
		{
			return CachedRegistry;
		}
		// Registry was garbage collected, remove stale entry
		StructToRegistryCache.Remove(InStruct);
	}

	// Reverse lookup: iterate all registries and find one with matching ItemStruct
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	if (!DRSubsystem)
	{
		return nullptr;
	}

	TArray<UDataRegistry*> AllRegistries;
	DRSubsystem->GetAllRegistries(AllRegistries);

	for (UDataRegistry* Registry : AllRegistries)
	{
		if (Registry && Registry->GetItemStruct() == InStruct)
		{
			StructToRegistryCache.Add(InStruct, Registry);
			return Registry;
		}
	}

	return nullptr;
}

// Returns overall number of cached rows for the given row struct type
int32 FDalRegistryRow::GetRowsNum(const UScriptStruct* InStruct)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems);
	return CachedItems.Num();
}

// Returns raw pointer to cached item data by struct type and RowName, O(1) lookup, or nullptr
const uint8* FDalRegistryRow::GetRowByName(const UScriptStruct* InStruct, FName RowName)
{
	const UDataRegistry* Registry = GetRegistryForStruct(InStruct);
	if (!Registry)
	{
		return nullptr;
	}

	const uint8* OutItemMemory = nullptr;
	const UScriptStruct* OutItemStruct = nullptr;
	const FDataRegistryId ItemId(FDataRegistryType(Registry->GetRegistryType()), RowName);
	Registry->GetCachedItemRaw(OutItemMemory, OutItemStruct, ItemId);
	return OutItemMemory;
}

// Returns raw pointer to cached item data at specified index for the given struct type, or nullptr
const uint8* FDalRegistryRow::GetRowByIndex(const UScriptStruct* InStruct, int32 Index)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return nullptr;
	}

	int32 CurrentIndex = 0;
	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		if (CurrentIndex == Index)
		{
			return Pair.Value;
		}
		++CurrentIndex;
	}
	return nullptr;
}

// Returns the row name at specified index for the given struct type, or NAME_None
FName FDalRegistryRow::GetRowNameByIndex(const UScriptStruct* InStruct, int32 Index)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return NAME_None;
	}

	int32 CurrentIndex = 0;
	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		if (CurrentIndex == Index)
		{
			return Pair.Key.ItemName;
		}
		++CurrentIndex;
	}
	return NAME_None;
}

// Iterates all cached items for the given struct type, calling Callback with raw item data
void FDalRegistryRow::ForEachRow(const UScriptStruct* InStruct, const TFunctionRef<void(const uint8*)>& Callback)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return;
	}

	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		Callback(Pair.Value);
	}
}

// Iterates all cached items for the given struct type, calling Callback with item name and raw data
void FDalRegistryRow::ForEachRowWithName(const UScriptStruct* InStruct, const TFunctionRef<void(FName ItemName, const uint8*)>& Callback)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return;
	}

	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		Callback(Pair.Key.ItemName, Pair.Value);
	}
}

// Finds first cached item matching predicate, or nullptr
const uint8* FDalRegistryRow::GetRowByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return nullptr;
	}

	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		if (Predicate(Pair.Value))
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

// Returns the row name of first cached item matching predicate, or NAME_None
FName FDalRegistryRow::GetRowNameByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	TMap<FDataRegistryId, const uint8*> CachedItems;
	if (!DalRegistryRowInternal::GetAllCachedItems(GetRegistryForStruct(InStruct), CachedItems))
	{
		return NAME_None;
	}

	for (const TPair<FDataRegistryId, const uint8*>& Pair : CachedItems)
	{
		if (Predicate(Pair.Value))
		{
			return Pair.Key.ItemName;
		}
	}
	return NAME_None;
}

// Counts cached items matching predicate
int32 FDalRegistryRow::CountRowsByPredicate(const UScriptStruct* InStruct, const TFunctionRef<bool(const uint8*)>& Predicate)
{
	int32 Count = 0;
	ForEachRow(InStruct, [&Count, &Predicate](const uint8* Data)
	{
		if (Predicate(Data))
		{
			++Count;
		}
	});
	return Count;
}

// Returns all Data Registries whose ItemStruct is a child of InBaseStruct
void FDalRegistryRow::ForEachRegistry(const UScriptStruct* InBaseStruct, const TFunctionRef<void(UDataRegistry* Registry, const UScriptStruct* ItemStruct)>& Callback)
{
	const UDataRegistrySubsystem* DRSubsystem = UDataRegistrySubsystem::Get();
	if (!DRSubsystem)
	{
		return;
	}

	TArray<UDataRegistry*> AllRegistries;
	DRSubsystem->GetAllRegistries(AllRegistries);

	for (UDataRegistry* Registry : AllRegistries)
	{
		const UScriptStruct* ItemStruct = Registry ? Registry->GetItemStruct() : nullptr;
		if (ItemStruct
		    && ItemStruct->IsChildOf(InBaseStruct))
		{
			Callback(Registry, ItemStruct);
		}
	}
}
